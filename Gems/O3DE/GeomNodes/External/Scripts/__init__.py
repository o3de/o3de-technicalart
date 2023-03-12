import sys
import os
import bpy
import logging as _logging

FRMT_LOG_LONG = "[%(name)s][%(levelname)s] >> %(message)s (%(asctime)s; %(filename)s:%(lineno)d)"

_PACKAGENAME = __name__
if _PACKAGENAME == '__main__':
    _PACKAGENAME = 'GeomNodes.External.Scripts'

# set up module logging
for handler in _logging.root.handlers[:]:
    _logging.root.removeHandler(handler)
_LOGGER = _logging.getLogger(_PACKAGENAME)
#_logging.basicConfig(format=FRMT_LOG_LONG, level=_logging.DEBUG)
_logging.basicConfig(level=_logging.DEBUG)
_LOGGER.debug('Initializing: {0}.'.format({_PACKAGENAME}))
#sys.stdout = open("f:/output.txt", "w")

dir = os.path.dirname(__file__)
if not dir in sys.path:
    sys.path.append(dir)

params = sys.argv[sys.argv.index("--") + 1:]
_LOGGER.debug('exe path: ' + params[0])
_LOGGER.debug('uuid: ' + params[1])
_LOGGER.debug('pid: ' + str(os.getpid()))

if __name__ == "__main__":
    from GeomNodes import init, run
    init(params[0], params[1])

    # run our loop to watch for updates
    
    if not bpy.app.background:
        bpy.app.timers.register(run)
    else:
        run()

# Close the file
#sys.stdout.close()
#sys.stdout = sys.__stdout__

del _LOGGER