import os
import subprocess
import sys


def find_tests(theme):
    """Find all of the test files in the assets directory."""
    test_files = []

    for dirpath, dirnames, filenames in os.walk("/home/codio/workspace/assets/{}/" .format(theme)):
        for filename in filenames:
            if filename.endswith('.xml'):
                test_files.append(os.path.join(dirpath, filename))

    return test_files


def main():
    """Collect the tests for the current milestone and run them against the oracle.

    If there are extra credit tests, they can be run independently of the other tests
    by using the --extra or -e flags. To run specific tests, one can use the --specific
    or -s flags with the name of the test directory:

    Examples
    --------
    $  python3 run_tests.py t4m1
        $  python3 run_tests.py t4m1 -e
    $  python3 run_tests.py t4m1 -s SpringTests
    """
    theme = sys.argv[1]
    specific_tests = theme
    if len(sys.argv) > 2:
        if sys.argv[2] == "--extra" or sys.argv[2] == '-e':
            specific_tests += "_extracredit"
        if sys.argv[2] == "--specific" or sys.argv[2] == '-s':
            specific_tests += "/{}" .format(sys.argv[3])

    tests = find_tests(specific_tests)
    successful_tests = 0
    failed_tests = 0

    for test in sorted(tests):
        print('Running test {}: ' .format(test), end='')
        subprocess.check_output(['/home/codio/workspace/build/FOSSSim/FOSSSim', '-s', '{}' .format(test),
                                 '-d', '0', '-o', '/home/codio/workspace/test_output.bin'])

        oracle = subprocess.check_output(['/home/codio/workspace/oracle/FOSSSimOracle{}' .format(theme.upper()),
                                          '-s', '{}' .format(test), '-d', '0', '-i', '/home/codio/workspace/test_output.bin'],
                                          universal_newlines=True)

        if 'Overall success: Passed.' in oracle:
            successful_tests += 1
            print('Passed.')
        elif 'Overall success: Failed.' in oracle:
            failed_tests += 1
            print('Failed.')

    print('------------------------------------------------')
    print('Successful Tests: {}' .format(successful_tests))
    print('Failed Tests: {}' .format(failed_tests))
    print('------------------------------------------------')


main()
