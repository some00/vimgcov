from pathlib import Path
from pprint import pprint
import time


def foo():
    import sys
    sys.path.append(str(Path(__file__).parent / "_build"))
    import _foo
    return _foo


foo = foo()
paths = list(map(str, Path("/home/tkonya/src/lvgl/").rglob("*.gcno")))

start = time.time()
rv = foo.foo(paths, 32)
end = time.time()
print(f"32 process {end - start}")

start = time.time()
foo.foo(paths, 1)
end = time.time()
print(f"1 process {end - start}")

input()
pprint(rv)
