import sqlite3
import pathlib

# try to find supercollate.so
EXTSO=pathlib.Path('../build/libsupercollate.so')
if not EXTSO.is_file():
    EXTSO=pathlib.Path('../cmake-build-debug/libsupercollate.so')
if not EXTSO.is_file():
    EXTSO=pathlib.Path('../cmake-build-release/libsupercollate.so')
# crash if the extension is nowhere
assert EXTSO.is_file(), 'impossible to find %s' % EXTSO


# un test qui sert a rien
def test_bla():
    # youplaboum!
    assert(True)

# simple test of the depenc function
def test_extension_depenc():
    con = sqlite3.connect("file::memory:?cache=shared")
    con.enable_load_extension(True)
    con.load_extension(str(EXTSO))
    cur = con.cursor()
    res = cur.execute("SELECT depenc('/')")
    assert(res.fetchone()[0]=='0000#')
    res = cur.execute("SELECT depenc('/aa/bb$3@/ccr*%')")
    assert(res.fetchone()[0]=='0003#/aa/bb$3@/ccr*%')
    con.close()


