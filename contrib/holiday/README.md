calcurse-holiday
================

`calcurse-holiday` is a Python script that can create holiday entries for
calcurse for a desired list of years based on a configuration file. Please note
that this script is still in an alpha version, so please report bugs (I think
the best way for this will be github, you can mention @atticus-sullivan)

Why not simply use reocuring events?
------------------------------------
Since some holiday days are not on the same date each year (e.g.some are
connected to the date Eastersunday is placed on), it is not possible to create
calcurse native reocuring events in calcurse for these events.

Usage
-----
* Create a `yaml` file which will be the base of which holiday dates will be
inserted.
* Then run `calcurse-holiday` with the path to the yaml-file you just created
  and a list of years you'd like to create the holiday-events for. (Duplicates
  will be omitted)

To check if the dates are correct before inserting them, just run the script
with the `-s` option.

**Tipp:** On failiure the new events should simple by at the end of your `app` file
of calcurse and it should be easy to remove them again

Example:
--------
```yaml
holiday.yaml
----------------------
# specify the description/name of the events that will be created
holiday_msg: "%s - free day" # %s will be replaced with the name/key of the
holiday date

# current valid date specs:
#    "easter [+-]x day" => Date will be x days before/after the day of Eastersunday
#    "MM-DD [+-]x weekday" => Date will be the x.th mon/thu/... before/after
#       YYYY-MM-DD (with given year).
#       The weekday is specified as 1->Monday 2->Thuesday ...
irregular holiday:
    - Eastersunday: "easter +0 day"
    - Eastermonday: "easter +1 day"
    - Thanksgiving: "11-01 +3 4"
```

`calcurse-holiday holiday.yaml 2021 2022` will read the holiday template from the
`holiday.yaml` file and create the holiday events for the years `2021` and `2022`

Notes
-----
* Currently the purpose of this script is ONLY for events that are weekday offset
based or on eastern. More base dates are possible, just suggest them ;)

* Maybe one day this will support "normal" reoccuring Events too. But since this
can much better be done via calcurses native reoccuring events (which is much
better, since one doesn't need to remember to recreate the events), this is not
the main purpose of this script.

* The functionallity of avoiding duplicate entries might be added into native
  calcurse via an commandline option. If this is complete one can remove the
  calls to calcurse to check if an event is already added to calcurse
