#!/usr/bin/env python
import time
import datetime
import json
import requests
import os
import sys
import glob

from contextlib import contextmanager
from distutils.dir_util import copy_tree, remove_tree
from models import (Assignment, 
                    Session, 
                    TestSceneRun,
                    CONTEST_THEME,
                    CONTEST_MILESTONE,
                    MAX_RUNTIME_CONTEST,
                    ORACLE_RUNTIME_CONTEST)

from printing import *
from uuid import uuid4

def unix_timestamp():
    return int(time.time())

# the width of the terminal output. things are left-padded
# to hit this target width.
MAIN_WIDTH = 78

VALID_SCENE_EXTENSIONS = ['.xml']
VALID_MOVIE_EXTENSIONS = ['.mpeg', '.mpg', '.mov', '.mkv', '.avi', '.mp4']

def find(iterable):
    try:
        result = iterable.next()
    except StopIteration:
        result = None

    return result

@contextmanager
def chdir(d):
    old_cwd = os.getcwd()
    os.chdir(d)
    try:
        yield
    finally:
        os.chdir(old_cwd)

POINTS_LOST_PER_MINUTE = 1.0 / 6.0
EXTRA_CREDIT_POINTS = 15.0
TIMEZONE_DIFFERENCE = datetime.timedelta(hours=5)

# Get the url to send the results to
CODIO_AUTOGRADE_URL = os.environ["CODIO_AUTOGRADE_URL"]
CODIO_UNIT_DATA = os.environ["CODIO_AUTOGRADE_ENV"]
# The date and time format to use. The deadline date should conform
DATE_FORMAT = "%Y-%m-%dT%H:%M:%S"
DATE_FORMAT_CODIO = "%Y-%m-%dT%H:%M:%S.%fZ"

# Send the grade back to Codio
def send_grade( grade ):
    s = requests.Session()
    s.mount('https://', requests.adapters.HTTPAdapter(max_retries=3))
    r = s.get("{0}&grade={1}".format(CODIO_AUTOGRADE_URL, grade))
    parsed = json.loads(r.content)
    return parsed['code'] == 1

# gets complete date from codio unit data
def get_completed_date():
    unit_info = json.loads(CODIO_UNIT_DATA)
    date = unit_info["completedDate"]
    return datetime.datetime.strptime(date, DATE_FORMAT_CODIO)


def get_current_assignments(session, theme, milestone):
    # allow users to submit all assignments ever, even when they're late.
    #return session.query(Assignment).all()
    return Assignment.get_assignments(session, theme, milestone)

def fatal(msg):
    print_fatal(msg)
    sys.exit(1)

def fatal_cancel(msg):
    print_fatal(msg)
    cancel_submission()
    sys.exit(0)

def check_original_folder_for_fosssim(original_folder):
    '''
    Ensures that the directory about to be submitted is formatted as expected.
    '''
    if not os.path.exists(original_folder):
        fatal("Folder '{}' does not exist.".format(original_folder))

    if not os.path.exists(os.path.join(original_folder, 'FOSSSim')):
        fatal("Couldn't find 'FOSSSim' directory in the top level of the submission folder.")

def compile_submission(submission_folder):
    build_folder = os.path.join(submission_folder, 'build/')
    lib_folder = os.path.join(submission_folder, 'lib/')
    if not os.path.exists(build_folder):
        os.mkdir(build_folder)
        if not os.path.exists(build_folder):
            fatal("Build directory was not correctly copied into the submission folder.")

    with chdir(build_folder):
        os.system('cmake -DCMAKE_BUILD_TYPE=Release ..')
        compilation_result = os.system('make -j')

        if compilation_result > 0:
            fatal_cancel(submission_folder, "Compilation failed.")

        expected_binary_path = os.path.abspath(os.path.join("./FOSSSim", "FOSSSim"))
        if not os.path.isfile(expected_binary_path):
            fatal_cancel(submission_folder, "Binary executable wasn't found in '{0}'.".format(expected_binary_path))

    os.system("chmod -R 777 " + submission_folder)

    return expected_binary_path

def shorten_test_path(path):
    return '/'.join(path.split('/')[-3:])

def print_test(path):
    path = shorten_test_path(path)
    s = "    {0}".format(blue(path))

    # add some padding
    s += (' ' * (MAIN_WIDTH - len(path) - 10))

    sys.stdout.write(s)
    sys.stdout.flush()

def print_result(result):
    if result is True:
        sys.stdout.write(bold(green("[ OK ]\n")))
    elif result is False:
        sys.stdout.write(bold(  red("[FAIL]\n")))
    elif result is None:
        sys.stdout.write(bold(      "[N/A ]\n"))
        
def print_result_contest(result, timing):
    if result is True:
        sys.stdout.write(bold(green("[ OK ] - " + str(timing) + " secs\n")))
    elif result is False:
        sys.stdout.write(bold(  red("[FAIL]\n")))
    elif result is None:
        sys.stdout.write(bold(      "[N/A ]\n")) 

def user_wants_to_run_extra_tests():
    return "y" in raw_input("Run Extra Tests ? [y/n]: ")

def run_tests(submission_executable, assignment, hashstr):
    tests = assignment.tests()
    runs = []

    print("")
    print("=" * MAIN_WIDTH)
    print("  Running Scenes:")
    print("=" * MAIN_WIDTH)
    print("")

    required_tests = [t for t in tests if not t.extra_credit]
    extra_credit_tests = [t for t in tests if t.extra_credit]

    for t in required_tests:
        print_test(t.filepath)
        result, et = t.run(submission_executable, assignment.oracle_path, hashstr, assignment.is_contest(), assignment.is_robot())
        if result is not None:
            sys.stdout.write("[DONE]\n")
            runs.append(TestSceneRun(path=t.filepath, success=result, extra_credit=t.extra_credit, elapsed=et))

    if len(extra_credit_tests) > 0:
        for t in extra_credit_tests:
            print_test(t.filepath)

            result, et = t.run(submission_executable, assignment.oracle_path, hashstr, assignment.is_contest(), assignment.is_robot())

            if result is not None:
                sys.stdout.write("[DONE]\n")
                runs.append(TestSceneRun(path=t.filepath, success=result, extra_credit=t.extra_credit, elapsed=et))
    else:
        for t in extra_credit_tests:
            runs.append(TestSceneRun(path=t.filepath, success=False, extra_credit=t.extra_credit, elapsed=0.0))

    return runs

def lateness(due_date):
    due_time = due_date + TIMEZONE_DIFFERENCE
    if datetime.datetime.utcnow() < due_time:
        return 0.0
    else:
        return (datetime.datetime.utcnow() - due_time).total_seconds() / 60.0

def print_passed_graded(percentage, due_date, passed_runs, total_runs, extra_credit_passed, total_extra_credit):
    lateness_percentage = lateness(due_date) * POINTS_LOST_PER_MINUTE
    percentage = percentage * (100.0 - lateness_percentage) / 100.0

    print("Passed {} / {} required tests and {} extra credit tests for a grade of {}% (lateness penalty {}% included).".format(
        bold(green(str(passed_runs))), 
        bold(str(total_runs)),
        bold(green(str(extra_credit_passed))),
        bold(blue(str(percentage))),
        bold(red(str(lateness_percentage)))))

    return max(percentage, 0.0)
                         
def print_passed(due_date, passed_runs, total_runs, extra_credit_passed=0, total_extra_credit=0):
    if (total_runs == 0): 
        percentage = 100.0
    else:
        percentage = 100 * round(float(passed_runs) / total_runs, 3)

    if total_extra_credit > 0:
        percentage += round(EXTRA_CREDIT_POINTS * float(extra_credit_passed) / total_extra_credit, 3)
    
    return print_passed_graded(percentage, due_date, passed_runs, total_runs, extra_credit_passed, total_extra_credit)

def print_test_summary(due_date, results):
    print("")
    print("=" * MAIN_WIDTH)
    print("  Results:")
    print("=" * MAIN_WIDTH)
    print("")
    for t in results:
        print_test(t.scene_path)
        if t.success is not None:
            print_result(t.success)

    required_tests = [r for r in results if not r.extra_credit]
    successful_required_tests = [r for r in required_tests if r.success]

    extra_credit_tests = [r for r in results if r.extra_credit]
    successful_extra_credit_tests = [r for r in extra_credit_tests if r.success]
    print("")
    print("=" * MAIN_WIDTH)
    print("")


    grade = 0.0
    if len(results) > 0:
        grade = print_passed(due_date,
                     len(successful_required_tests), 
                     len(required_tests), 
                     len(successful_extra_credit_tests), 
                     len(extra_credit_tests))

    print("")
    return grade

def print_test_summary_contest(due_date, results):
    print("")
    print("=" * MAIN_WIDTH)
    print("  Results:")
    print("=" * MAIN_WIDTH)
    print("")
    
    required_tests = [r for r in results if not r.extra_credit]
    successful_required_tests = [r for r in required_tests if r.success]

    extra_credit_tests = [r for r in results if r.extra_credit]
    successful_extra_credit_tests = [r for r in extra_credit_tests if r.success]
                                
    grade = 0.0
    grade_each = 100.0 / float(len(required_tests) + len(extra_credit_tests))
    for t in results:
        print_test(t.scene_path)
        if t.success is not None:
            print_result_contest(t.success, t.elapsed)
            grade = grade + grade_each * pow(max((MAX_RUNTIME_CONTEST - t.elapsed) / (MAX_RUNTIME_CONTEST - ORACLE_RUNTIME_CONTEST), 0.0), 1.0 / 3.0)
    
    print("")
    print("=" * MAIN_WIDTH)
    print("")
    
    if len(results) > 0:
        grade = print_passed_graded(grade, due_date,
                     len(successful_required_tests), 
                     len(required_tests), 
                     len(successful_extra_credit_tests), 
                     len(extra_credit_tests))
                         
    print("")
    return grade

def user_wants_to_submit():
    return "y" in raw_input("Submit [y/n]: ")


def get_valid(msg, parser=lambda s: int(s)):
    result = None
    while result is None:
        try:
            result_str = raw_input(msg)
            result = parser(result_str)
        except ValueError:
            print("{} is not a valid response.".format(result_str))

    return result

def submit_assignment(ses, original_folder, assignment):
    results = []

    submission_executable = compile_submission(original_folder)

    try:
        results = run_tests(submission_executable, assignment, uuid4().hex)
    except:
        print_fatal("Python Error while running tests.")
        cancel_submission()
        raise
    
    if assignment.is_contest() is True:
        grade = print_test_summary_contest(assignment.due_date, results)
    else:
        grade = print_test_summary(assignment.due_date, results)

    res = send_grade(grade)
    exit( 0 if res else 1)

def get_assignment(ses, theme, milestone):
    assignments = get_current_assignments(ses, theme, milestone)
    if len(assignments) == 0:
        print("No assignments are available to submit.")
        sys.exit(0)

    # Which open assignment does the user intend to submit?
    return assignments[0]

def process_submission(theme, milestone, original_folder):
    '''
    The main function, that takes a student's UNI and a path to the submitted
    folder, and runs test scenes, calculates a grade, and adds rows to the
    database for this submission (unless the user cancels submission).
    '''
    ses = Session()

    check_original_folder_for_fosssim(original_folder)

    assignment = get_assignment(ses, theme, milestone)

    submit_assignment(ses, original_folder, assignment)

def cancel_submission():
    print("Submission canceled.")


def main():
    process_submission(sys.argv[2], sys.argv[3], sys.argv[1])

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print("{} <Submission Directory> <THEME> <MILESTONE>".format(sys.argv[0]))
        sys.exit(1)

    main()
