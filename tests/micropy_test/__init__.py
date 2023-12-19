"""
The test script will only run the one test on microptyhon. It is not a
framework that can be used for other tests. This has a reduced code footprint.
entrypoint API is almost the same as the framework.


Requirements
Python >= 3.10

apng library
pillow library

dd the following to the CI prior to the test running

python3 -m pip install pillow, apng

Example command line to run the test. I suggest doing this from the root of the
binding directory. It is just a simple location to do it from.

Paths that are passed in MUST be relitive to the current working directory.
python3 lib/lv_bindings/lvgl/tests/micropy_test/__init__.py --artifact-path=lib/lv_bindings/lvgl/tests/micropy_test/artifacts --mpy-path=ports/unix/build-standard/micropython

"""
import os
import apng
import time
import signal
import argparse
import binascii
import unittest
import threading
import subprocess

from io import BytesIO
from PIL import Image as Image


DEBUG = 0

debug_log = None


def log(*args):
    args = ' '.join(repr(arg) for arg in args)
    debug_log.write(args + '\n')

    if DEBUG:
        sys.stdout.write('\033[31;1m' + args + '\033[0m\n')
        sys.stdout.flush()


BASE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__)))
TEST_PATH = os.path.join(BASE_PATH, 'micropy.py')
IMG_PATH = os.path.join(BASE_PATH, '../ref_imgs/binding.png')

CTRL_C = b'\x03'  # 2 times to exit any running code
CTRL_D = b'\x04'  # exit paste mode committing and running what has been pasted
CTRL_E = b'\x05'  # enter paste mode

PASTE_PROMPT = b'==='
REPL_PROMPT = b'>>>'

os.environ['MICROPYINSPECT'] = '1'


class TestData:

    def __init__(self):
        self.watchdog_timer = time.time()
        self.result = None
        self.error_data = ''
        self.event = threading.Event()


class MicroPython_Test(unittest.TestCase):
    # these are here simply to make an IDE happy. Their values get dynamically
    # set when the class gets constructed
    process: subprocess.Popen = None
    exit_event: threading.Event = None

    @classmethod
    def send(cls, cmd):
        if cls.process is None:
            return

        log('<---', cmd)
        cls.process.stdin.write(cmd)
        cls.process.stdin.flush()

    @classmethod
    def read_until(cls, marker):
        micropy_data = b''
        error_data = b''

        log('MARKER', marker)

        logged = False

        while (
            not micropy_data.endswith(marker) and
            not cls.exit_event.is_set()
        ):
            try:
                char = cls.process.stdout.read(1)
            except:  # NOQA
                break

            if char:
                micropy_data += char
                logged = False
            else:
                logged = True
                log('--->', micropy_data)

            if micropy_data.endswith(b'\nERROR END\n'):
                error_data = micropy_data.split(b'\nERROR START\n')[-1].split(b'\nERROR END\n')[0]
                micropy_data = b''
                log('---> ERROR: ', error_data)
                logged = True
                break

        if not logged:
            log('--->', micropy_data)

        if cls.exit_event.is_set():
            log('--EXIT EVENT SET')

        if micropy_data:
            return micropy_data.replace(marker, b''), error_data

        return micropy_data, error_data

    @classmethod
    def setUpClass(cls):
        os.chdir(os.path.dirname(__file__))
        cls.exit_event = threading.Event()

        log(f'--SETTING UP')
        cls.process = subprocess.Popen(
            ['bash'],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=subprocess.PIPE,
            env=os.environ,
            shell=True,
            preexec_fn=os.setsid
        )
        log(f'--RUNNING MICROPYTHON ({MICROPYTHON_PATH})')

        cls.send(b'cd ' + os.path.dirname(__file__).encode('utf-8') + b'\n')
        cls.send(MICROPYTHON_PATH.encode('utf-8') + b'\n')
        _, error_data = cls.read_until(b'>>>')

        if error_data:
            raise RuntimeError(error_data)

        log('--MICROPYTHON STARTED')

    @classmethod
    def tearDownClass(cls):
        log('--TEARDOWN STARTED')

        if cls.process is not None:
            cls.send(CTRL_C)
            cls.send(CTRL_C)
            cls.send(CTRL_D)

            if not cls.process.stdin.closed:
                cls.process.stdin.close()

            os.killpg(os.getpgid(cls.process.pid), signal.SIGTERM)

            cls.process.wait()

            if not cls.process.stdout.closed:
                cls.process.stdout.close()

            if not cls.process.stderr.closed:
                cls.process.stderr.close()

            cls.process = None

        log(f'--TEARDOWN FINISHED')

    def test_1_image_compare(self):
        image = Image.open(IMG_PATH)

        with open(TEST_PATH, 'rb') as f:
            test_code = f.read()

        log(f'--RUNNING TEST ({TEST_PATH})')

        test_code = test_code.strip()

        if self.__class__.process is None:
            self.fail('MicroPython failure.')

        self.send(CTRL_E)

        _, error_data = self.read_until(PASTE_PROMPT)
        if error_data:
            self.fail(error_data)

        test_code = test_code.split(b'\r\n')

        for i, line in enumerate(test_code):
            self.send(line + b'\n')
            _, error_data = self.read_until(b'\n')

            if error_data:
                self.fail(error_data)

            time.sleep(0.002)

        # self.read_until(b'# end\n')

        def _do(td: TestData):
            self.send(CTRL_D)
            td.error_data = b''
            td.watchdog_timer = time.time()

            curr_time = capture_start = time.time()
            td.result = []

            while (
                (curr_time - capture_start) * 1000 <= 10000 and
                not self.__class__.exit_event.is_set()
            ):
                try:
                    _, td.error_data = self.read_until(b'FRAME START\n')
                    if td.error_data:
                        break

                    fd = []

                    lne, td.error_data = self.read_until(b'\n')
                    if td.error_data:
                        break

                    while lne != b'FRAME END':
                        td.watchdog_timer = time.time()
                        fd.append(lne)
                        lne, td.error_data = self.read_until(b'\n')

                        if td.error_data:
                            break
                        if self.__class__.exit_event.is_set():
                            break

                    td.result.append(fd[:])

                    if td.error_data:
                        break
                    if self.__class__.exit_event.is_set():
                        break

                except:  # NOQA
                    import traceback

                    traceback.print_exc()

                    td.error_data = traceback.format_exc()
                    return

                curr_time = time.time()

            td.event.set()

        test_data = TestData()

        t = threading.Thread(
            target=_do,
            args=(test_data,)
        )
        t.daemon = True
        test_data.watchdog_timer = time.time()

        t.start()

        while (
            (time.time() - test_data.watchdog_timer) * 1000 <= 10000 and
            not test_data.event.is_set()
        ):
            test_data.event.wait(0.05)

        self.send(CTRL_C)
        self.send(CTRL_C)

        if not test_data.event.is_set():
            self.__class__.exit_event.set()
            # self.read_until(REPL_PROMPT)

        width = 800
        height = 480

        data = test_data.result[:]
        test_data.result = []

        for frame_data in data:
            try:
                frame = bytearray(
                    b''.join(binascii.unhexlify(lne) for lne in frame_data)
                )

                # I don't exactly know why the byte order is backwards but it is
                frame = bytes(bytearray([
                    item for j in range(0, len(frame), 3)
                    for item in [frame[j + 2], frame[j + 1], frame[j]]
                ]))

                img = Image.new('RGB', (width, height))
                img.frombytes(frame)
                test_data.result.append(img)
            except:  # NOQA
                test_data.result.append(None)
                continue

        if not os.path.exists(ARTIFACT_PATH):
            os.mkdir(ARTIFACT_PATH)

        a_png_path = os.path.join(ARTIFACT_PATH, 'result.apng')

        def save_apng():
            try:
                artifact.save(a_png_path)
            except IndexError:
                pass
            except:
                import traceback

                traceback.print_exc()

        artifact = apng.APNG()
        res_img = image.convert('RGB')
        image.close()
        res_data = list(res_img.getdata())
        res_img.close()

        for frame_num, im in enumerate(test_data.result):
            try:
                img = im.convert('RGB')
                im.close()
                byte_data = list(img.getdata())
                writer = BytesIO()
                img.save(writer, 'PNG')
                img.close()

                writer.seek(0)
                png = apng.PNG.from_bytes(writer.read())
                artifact.append(png)

                img.save(os.path.join(ARTIFACT_PATH, f'frame{frame_num}.png'), 'PNG')

                with open(os.path.join(ARTIFACT_PATH, f'frame{frame_num}.bin'), 'wb') as f:
                    # have to flatten the data and remove the alpha
                    # from the PIL image it is formatted as
                    # [(r, g, b), (r, g, b)]
                    f.write(bytes(bytearray([
                        item for sublist in byte_data
                        for item in sublist
                    ])))

                if res_data == byte_data:
                    save_apng()
                    self.assertEqual(res_data, byte_data, 'Frames do not match')
            except:  # NOQA
                import traceback

                if not test_data.error_data:
                    test_data.error_data = traceback.format_exc()

                break

        save_apng()

        if test_data.error_data:
            log(f'--TEST FAILED')
            log(test_data.error_data)
            self.fail(test_data.error_data)

        self.fail('Frame Data does not match')


cwd = os.path.abspath(os.getcwd())

ARTIFACT_PATH = os.path.join(cwd, 'artifacts')
MICROPYTHON_PATH = os.path.join(cwd, 'micropython')


if __name__ == '__main__':
    import sys

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument(
        '--artifact-path',
        dest='artifact_path',
        help='path to save artifacts to',
        action='store'
    )
    arg_parser.add_argument(
        '--mpy-path',
        dest='mpy_path',
        help='path to micropython',
        action='store'
    )
    arg_parser.add_argument(
        '--debug',
        dest='debug',
        help='debug output',
        action='store_true'
    )

    args = arg_parser.parse_args()

    ARTIFACT_PATH = os.path.join(cwd, args.artifact_path)
    MICROPYTHON_PATH = os.path.join(cwd, args.mpy_path)
    DEBUG = args.debug

    if not os.path.exists(ARTIFACT_PATH):
        raise RuntimeError(f'Artifact path does not exist ({ARTIFACT_PATH})')

    if not os.path.exists(MICROPYTHON_PATH):
        raise RuntimeError(f'MicroPython binary not found ({MICROPYTHON_PATH})')

    debug_log_path = os.path.join(ARTIFACT_PATH, 'debug.log')
    debug_log = open(debug_log_path, 'w')

    unittest.main(argv=[sys.argv[0], '-v'])

    debug_log.close()
    print(f'View the debug output in "{debug_log_path}"')