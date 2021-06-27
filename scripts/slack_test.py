import sys
from slacker import Slacker
import os
import subprocess


slack = Slacker('')

response = slack.users.list()

members = response.body['members']

hash = os.environ.get('GIT_COMMIT')  # 93b7dfbe0b9e698a1de666561a1fe31ffb9cc8cb
url = os.environ.get('RUN_DISPLAY_URL')

if not hash:
    hash = ''

if not url:
    url = ''
else:
    url = url.rstrip('display/redirect') + "/console"

git_process = subprocess.Popen(['git', 'log', '-1', hash], stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
out, err = git_process.communicate()

out = out.decode("utf-8")

outsplit = out.split('\n')

date = None
email = None

for x in outsplit:
    if 'Date:' in x:
        date = x
    if 'Author:' in x:
        email = x.split("<")[1].split(">")[0]


if email is None:
    print("Email was not found, something wrong with git or env")


kids = dict()

for w in members:
    kids[w.get('profile').get('email')] = w.get('name')

premsg = '*BUILD PASSED*\n'

for arg in sys.argv[1:]:
    print (arg)
    if "FAILED" in arg:
        premsg = '*BUILD FAILED!!!*\n'


with open('custodians.txt') as f:
    msg = '%s>Author: %s \n>Commit: %s \n>%s\nBUILD URL: %s\n\n\n' % (premsg, kids[email], hash, date, url)
    lines = f.readlines()
    for line in lines:
        slack.chat.post_message('@%s' % kids[line], msg, username='Jenkins')

if not email in lines:
    msg = '%s>Commit: %s \n>%s\nBUILD URL: %s\n\n\n' % (premsg, hash, date, url)
    slack.chat.post_message('@%s' % kids[email], msg, username='Jenkins')