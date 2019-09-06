import os, sys, time, subprocess, sqlite3, re
from pathlib import Path
try:
    home = str(Path.home())
except:
    home = os.getenv("HOME")
sys.path.insert(1, home+ '/UGP/eagle')
from daemon import Daemon


class EagleLiveHostsDaemon(Daemon):
    def __init__(self, home, db , livehosts, tmplivehosts, script, hostname, pidfile, stdout='/dev/null', stderr='/dev/null', stdin='/dev/null'):
        super().__init__(pidfile, stdout, stderr , stdin )
        self.home = home
        self.db = db
        self.livehosts = livehosts
        self.script = script
        self.hostname = hostname
        self.tmplivehosts = tmplivehosts

    def run(self):
        while True:
            open(self.stdout, 'w').close()
            # open(self.stderr, 'w').close()
            subprocess.run([self.script, self.livehosts, self.tmplivehosts], stdout=subprocess.PIPE)
            time.sleep(180)

if __name__ == "__main__":

    output = subprocess.run(["hostname"], stdout=subprocess.PIPE)
    hostname = output.stdout.decode("utf-8").strip(" \n")

    if not os.path.isdir(home + "/.eagle/" + hostname):
        os.makedirs(home + "/.eagle/" + hostname, exist_ok=True)

    pidfilename = home + "/.eagle/" + hostname + "/livehosts.pid"
    stdout = home + "/.eagle/" + hostname + "/livehosts.log"
    stderr = home + "/.eagle/" + hostname + "/livehosts.err"
    db = home + "/.eagle/data.db"
    livehosts = home + "/.eagle/livehosts.txt"
    tmplivehosts = home + "/.eagle/" + hostname + "/livehosts.txt"
    script= home + "/UGP/eagle/livehosts/livehosts.sh"

    daemon = EagleLiveHostsDaemon(home, db, livehosts,tmplivehosts, script, hostname, pidfilename, stdout, stderr)
    if len(sys.argv) == 2:
        if 'start' == sys.argv[1]:
            print("livehosts Daemon on" + hostname + " : Starting")
            daemon.start()
        elif 'stop' == sys.argv[1]:
            print("livehosts Daemon on" + hostname + " : Stoping")
            daemon.stop()
        elif 'restart' == sys.argv[1]:
            print("livehosts Daemon on" + hostname + " : Restarting")
            daemon.restart()
        else:
            print("Unknown command")
            sys.exit(2)
        sys.exit(0)
    else:
        print("usage: " + sys.argv[0] + "  start|stop|restart")
        sys.exit(2)
