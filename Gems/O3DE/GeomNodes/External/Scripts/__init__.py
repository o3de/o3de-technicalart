import sys
import os
import bpy
import json

sys.stdout = open("f:/output.txt", "w")

dir = os.path.dirname(__file__)
if not dir in sys.path:
    sys.path.append(dir)

params = sys.argv[sys.argv.index("--") + 1:]
print('exe path: ' + params[0])
print('uuid: ' + params[1])
print('pid: ' + str(os.getpid()))

if __name__ == "__main__":
    from GeomNodes import init, run
    init(params[0], params[1])

    # run our loop to watch for updates
    
    if not bpy.app.background:
        bpy.app.timers.register(run)
    else:
        run()

# Close the file
sys.stdout.close()
sys.stdout = sys.__stdout__