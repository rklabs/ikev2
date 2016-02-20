import re
import click
import subprocess

@click.command()
@click.option('--findstr', help='find string')
@click.option('--replacestr', help='replace string')
def findAndReplace(findstr, replacestr):
    if not findstr or not replacestr:
        print "try using --help"
        return

    grepProc = subprocess.Popen("grep %s *" % findstr,
                            shell=True,
                            stdout=subprocess.PIPE)

    grepProc.wait()
    output = grepProc.communicate()[0]

    fileList = []
    for line in output.split("\n"):
        if line:
            fileList.append(line.split(":")[0])

    for fl in fileList:
        sedProc = subprocess.Popen("sed -i 's/%s/%s/g' %s" %
                                   (findstr, replacestr, fl),
                                    shell=True,
                                    stdout=subprocess.PIPE)
        sedProc.wait()

        print 'Replaced {0} in {1}'.format(findstr, fl)

        output = sedProc.communicate()[0]

if __name__ == '__main__':
    findAndReplace()

