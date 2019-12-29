import subprocess

revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip().decode('ascii')
print("-DGIT_TAG=%s" % revision)
