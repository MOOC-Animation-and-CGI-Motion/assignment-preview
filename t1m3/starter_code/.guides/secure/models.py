#!/usr/bin/env python

import datetime
import os
import sys
import signal
import tempfile
import time

from subprocess import Popen, PIPE, STDOUT
from sqlalchemy import create_engine, Column, Integer, String, DateTime, ForeignKey, Boolean, Text
from sqlalchemy.orm import sessionmaker, relationship
from sqlalchemy.ext.declarative import declarative_base
from os import environ

from printing import *

DATABASE_FILEPATH = "/home/codio/workspace/.guides/secure/grade.db"
STD_LIB_DIRPATH = "/home/codio/workspace/.guides/secure/libs"
TEST_EXTENSION = ".xml"
CONTEST_THEME = 2
CONTEST_MILESTONE = 3 # T2M3 is for the contest
ROBOT_THEME = 5 # T5MX for robot
FLUID_THEME = 6
FINAL_THEME = 7
ORACLE_RUNTIME = 8.0
MAX_RUNTIME = 8.0
MAX_RUNTIME_CONTEST = 60.0
ORACLE_RUNTIME_CONTEST = 1.0

engine = create_engine('sqlite:///{0}'.format(DATABASE_FILEPATH))
Session = sessionmaker(bind=engine)

Base = declarative_base()

class Alarm(Exception):
    pass

def alarm_handler(signum, frame):
    raise Alarm

class TestScene(object):
    def __init__(self, filepath, graded=True, hidden=False, extra_credit=False):
        self.filepath = filepath
        self.graded = graded
        self.hidden = hidden
        self.extra_credit = extra_credit

    # time limit for the simulation to terminate, or mark it as time out.
    def run_student_code(self, submission_binary, output_filepath, timeout):
        signal.signal(signal.SIGALRM, alarm_handler)
        signal.alarm(int(timeout)) 
        elapsed = float(timeout)
        try:
            env = dict(os.environ)
            env['LD_PRELOAD'] = '/lib/x86_64-linux-gnu/libpthread.so.0'
            tstart = time.time()
            process = Popen([submission_binary, "-s", self.filepath, "-d", "0", "-o", output_filepath], stdout=PIPE, stderr=STDOUT, env=env)
            out, err = process.communicate()
            elapsed = float(time.time() - tstart)
            result_code = process.returncode
            signal.alarm(0) 
        except Alarm:
            sys.stdout.write(bold(red("[TIME]\n")))
            print("Student executable took longer than {} seconds to execute.".format(timeout))

        if result_code != 0:
            sys.stdout.write(bold(red("[N/A ]\n")))
            print(err)
            print("Student executable crashed (exit code {}).".format(result_code, submission_binary))

        if not os.path.isfile(output_filepath):
            sys.stdout.write(bold("[N/A ]\n"))
            print(err)
            print("Failed to generate output file '{}'.".format(output_filepath))
        return elapsed

    def run_student_robot_code(self, submission_binary, timeout):
        signal.signal(signal.SIGALRM, alarm_handler)
        signal.alarm(int(timeout))
        elapsed = timeout
        try:
            env = dict(os.environ)
            env['LD_PRELOAD'] = '/lib/x86_64-linux-gnu/libpthread.so.0'
            tstart = time.time()
            process = Popen([submission_binary, "-s", self.filepath, "-d", "0"], stdout=PIPE, stderr=STDOUT, env=env)
            out, err = process.communicate()
            elapsed = float(time.time() - tstart)
            result_code = process.returncode

            signal.alarm(0)
        except Alarm:
            sys.stdout.write(bold(red("[TIME]\n")))
            print("Student executable took longer than {} seconds to execute.".format(timeout))

        if result_code != 0:
            sys.stdout.write(bold(red("[N/A ]\n")))
            print(err)
            print("Student executable crashed (exit code {}).".format(result_code, submission_binary))
        if "Overall success: Passed" in out:
            return True, elapsed
        elif "Overall success: Failed" in out:
            print(out)
            return False, elapsed
        else:
            return None, elapsed

    def grade_student_code(self, oracle_binary, output_filepath):
        if not os.path.isfile(oracle_binary):
            sys.stdout.write(bold("[N/A ]\n"))
            raise Exception("Failed to open oracle '{}'.".format(oracle_binary))
    
        signal.signal(signal.SIGALRM, alarm_handler)
        signal.alarm(int(ORACLE_RUNTIME))

        try:
            # run the oracle to grade the output file
            env = dict(os.environ)
            env['LD_PRELOAD'] = '/lib/x86_64-linux-gnu/libpthread.so.0'
            out, err = Popen([oracle_binary, "-s", self.filepath, "-d", "0", "-i", output_filepath], stdout=PIPE, env=env).communicate()

            signal.alarm(0)
        except Alarm:
            sys.stdout.write(bold(blue("[TIME]\n")))
            raise Exception("Oracle took longer than {} seconds to check result.".format(ORACLE_RUNTIME))

        if "Overall success: Passed" in out:
            # print("Passed")
            return True
        elif "Overall success: Failed" in out:
            print("")
            print(out)
            return False
        else:
            print("")
            print(out)
            return None

    def run(self, submission_binary, oracle_binary, hashstr, is_contest, is_robot):
        # the file used to
        output_file = tempfile.NamedTemporaryFile()

        timeout = MAX_RUNTIME
        if is_contest is True:
            timeout = MAX_RUNTIME_CONTEST

        result = None
        elapsed = float(timeout)

        try:
            if is_robot is True:
                result, elapsed = self.run_student_robot_code(submission_binary, timeout)
            else:
                elapsed = self.run_student_code(submission_binary, output_file.name, timeout)
                result = self.grade_student_code(oracle_binary, output_file.name)
        except Exception as e:
            print(e)
            return None, float(timeout)

        return result, elapsed

class Assignment(Base):
    __tablename__ = "assignments"

    id  = Column(Integer, primary_key=True)

    theme       = Column(Integer)
    milestone   = Column(Integer)
    deliverable = Column(Integer)

    oracle_path = Column(String)
    
    due_date = Column(DateTime)

    directories = relationship("AssignmentAssetDirectory", backref='assignment')

    def __init__(self, **kwargs):
        self.set_dict(**kwargs)

    def set_dict(self, theme=None, milestone=None, deliverable=None,
            oracle_path=None, 
            due_date=None, directories=None):
        self.tests = None

        self.theme = theme
        self.milestone = milestone
        self.deliverable = deliverable
        self.oracle_path = oracle_path
        self.due_date = due_date

        if directories is not None:
            pass

    def __repr__(self):
        return "<Assignment t{}m{}d{}>".format(self.theme, self.milestone, self.deliverable)

    def __json__(self):
        return {
            "id":self.id,
            "theme":self.theme,
            "milestone":self.milestone,
            "deliverable":self.deliverable,
            "oracle_path":self.oracle_path,
            "due_date":self.due_date,
            "asset_directories":[d.__json__() for d in self.directories]
        }


    @staticmethod
    def get_late_window():
        return datetime.timedelta(hours=10)

    @staticmethod
    def get_assignments(session=None, theme_=1, milestone_=1):
        if session is None:
            session = Session()

        now = datetime.datetime.now()

        # duration of time after the official 'due date' during which time
        # you can still submit.
        late_window = Assignment.get_late_window()

        return session.query(Assignment).filter(Assignment.theme == theme_)\
                                        .filter(Assignment.milestone == milestone_)\
                                        .filter(Assignment.due_date >= now - late_window)\
                                        .all()

    def is_fluid(self):
        return self.theme == FLUID_THEME

    def is_final_project(self):
        return self.theme == FINAL_THEME

    def is_creative_scene(self):
        return False

    def is_contest(self):
        return self.theme == CONTEST_THEME and self.milestone == CONTEST_MILESTONE

    def is_robot(self):
        return self.theme == ROBOT_THEME

    def name(self):
        return "t{}m{}".format(self.theme, self.milestone)

    def __str__(self):
        return "<Assignment {} due on {}>:\n\t{}".format(self.name(),
                self.due_date,
                "\n\t".join(map(str, self.directories)))

    def graded_tests_count(self):
        return len(t for t in self.tests() if t.graded)

    def tests(self):
        tests = []

        for d in self.directories:
            tests += d.tests()

        return tests

class AssignmentAssetDirectory(Base):
    '''
    A directory of test assets (scene files) associated with an assignment.
    '''
    __tablename__ = "assignment_asset_directories"

    id  = Column(Integer, primary_key=True)

    assignment_id  = Column(Integer, ForeignKey('assignments.id'))

    path = Column(String(100))

    extra_credit = Column(Boolean)
    graded       = Column(Boolean)
    hidden       = Column(Boolean)

    def __init__(self, **kwargs):
        self.set_dict(**kwargs)

    def set_dict(self, path="", extra_credit=False, graded=True, hidden=False):
        self.extra_credit = extra_credit
        self.path = path
        self.graded = graded
        self.hidden = hidden

    def __json__(self):
        return {
            "id":self.id,
            "path":self.path,
            "extra_credit":self.extra_credit,
            "graded":self.graded,
            "hidden":self.hidden
        }

    def __str__(self):
        return "<AssignmentAssetDirectory '{}'{}{}>".format(self.path,
                " Graded" if self.graded else " Ungraded",
                " Hidden" if self.hidden else "")

    def tests(self):
        '''
        Returns an array of TestScenes, one for each scene file in this
        directory (and all its subdirectories).
        '''
        tests = []

        for root, dirnames, filenames in os.walk(self.path):
            for filename in filenames:
                if (os.path.splitext(filename)[1] == TEST_EXTENSION):
                    tests.append(
                        TestScene(os.path.join(root, filename),
                             graded=self.graded,
                             hidden=self.hidden,
                             extra_credit=self.extra_credit))

        return tests

class TestSceneRun():
    scene_path    = ""
    run_time      = datetime.datetime.utcnow
    success       = False
    extra_credit  = False
    elapsed       = 0.0

    def __init__(self, path="", success=None, extra_credit=False, elapsed=0.0):
        self.scene_path = path
        self.success = success
        self.extra_credit = extra_credit
        self.run_time = datetime.datetime.now()
        self.elapsed = elapsed

def main():
    Base.metadata.create_all(engine)

if __name__ == '__main__':
    main()
