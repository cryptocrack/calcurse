#!/usr/bin/env python3

import sys
import os
import subprocess
from datetime import datetime, timedelta
import argparse

import yaml
import re

# to calculate the date of the Eastersunday
def spencerFormula(y:int) -> datetime:
    a = y % 19
    b = y // 100
    c = y % 100
    d = b // 4
    e = b % 4
    f = (b+8) // 25
    g = (b-f+1) // 3
    h = (19*a+b-d-g+15) % 30
    i = c // 4
    k = c % 4
    l = (32+2*e+2*i-h-k) % 7
    m = (a+11*h+22*l) // 451
    n = (h+l-7*m+114) // 31
    p = (h+l-7*m+114) % 31

    return datetime(year=y, month=n, day=p+1)


def fullDayAppointmentIcal(date:datetime, offset:int, desc:str, noDup:bool = True, simu:bool = False) -> str:
    ret = []
    ret.append("BEGIN:VEVENT")
    ret.append("DTSTART;VALUE=DATE:" + (date + timedelta(days=offset)).strftime("%Y%m%d"))
    ret.append("SUMMARY:" + desc)
    if simu:
        print("Event", desc, "on ", (date + timedelta(days=offset)).strftime("%Y-%m-%d"), "would be created")
    ret.append("END:VEVENT")
    if noDup:
        command = [
                "/usr/bin/calcurse",
                "-Q",
                "-d", "%s" % (date + timedelta(days=offset)).strftime("%d/%m/%Y"),
                "--filter-pattern",'"^%s$"' % re.escape(desc).replace("\\\\", "\\")]
        p = subprocess.Popen(" ".join(command), stderr=None, stdin=None, stdout=subprocess.PIPE, encoding="utf-8", shell=True)
        stdout, stderr = p.communicate()
        if stdout != "":
            print(ret, "is already present -> skip")
            return ""

    return "\n".join(ret)


def toCalcurse(data:list[str]):
    if "".join(data) != "":
        data = \
            ["BEGIN:VCALENDAR\nVERSION:2.0\nPRODID:-//calcurse-holiday//NONSGML calcurse-holiday//EN"] \
            + data \
            + ["END:VCALENDAR\n"]

        command = ["calcurse"] + [
            '-i', '-', #read from stdin
            '--dump-imported',
            '-q' # quiet
            ]
        p = subprocess.Popen(command, stderr=None, stdin=subprocess.PIPE, stdout=None, encoding="utf-8")
        p.communicate("\n".join(data))


def createForYears(file:str, years:list[int], simu:bool):
    with open(file, 'r') as f:
        c = yaml.load(f, Loader=yaml.FullLoader)

    msg = c["holiday_msg"] if "holiday_msg" in c else "%s"

    if "irregular holiday" in c:
        ical = []
        for d in c["irregular holiday"]:
            d = list(d.items())
            if len(d) > 1:
                print("Error while parsing the config", file=sys.stderr)
                continue

            name,dateDesc = d[0]

            r1 = re.match(r'^easter ([+-]\d+) day$', dateDesc)
            r2 = re.match(r'^(\d{2}\-\d{2}) ([+-]\d+) ([1-7])$', dateDesc)

            for year in years:
                if r1:
                    offset = r1.groups()[0]
                    dtStart = spencerFormula(year)
                    ical.append(fullDayAppointmentIcal(dtStart, int(offset), msg % name, simu=simu))
                elif r2:
                    start,scale,weekday = r2.groups()
                    dtStart = datetime.strptime(str(year)+"-"+start, "%Y-%m-%d")
                    # print(int(weekday), "-", dtStart.weekday(),"- 1", "% 7 + ", int(scale), "* 7")
                    offset = (int(weekday) - dtStart.weekday()-1) % 7 + int(scale)*7
                    ical.append(fullDayAppointmentIcal(dtStart, offset, msg % name, simu=simu))
                else:
                    print("Failed to read entry", dateDesc, file=sys.stderr)
        if simu:
            print("\nThis ical (+header) would have been imported")
            print("\n".join(ical))
        else:
            toCalcurse(ical)


parser = argparse.ArgumentParser("calcurse-holiday",
        description="Add holiday entries based on given holiday.yaml file to calcurse")

parser.add_argument("file", nargs='?',
        default="./holiday.yaml",
        help="The yaml file to read the holidays from [defaults to './holiday.yaml']")
parser.add_argument("year", nargs='+',
        type=int,
        help="Specify a list of years to generate the holidays for")
parser.add_argument("-s", "--simulate",
        action='store_true',
        help="Only simulate the insertion of the events")

args = parser.parse_args()

createForYears(args.file, args.year, args.simulate)
