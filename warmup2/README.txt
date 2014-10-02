Documentation for Warmup Assignment 2
=====================================

+-------+
| BUILD |
+-------+

Comments: ?
make
./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]

All the commandline arguements are optional.

+------+
| SKIP |
+------+


+---------+
| GRADING |
+---------+


Basic running of the code : Yes

Missing required section(s) in README file : No
Cannot compile : No
Compiler warnings : none
"make clean" : Included
Segmentation faults : No
Separate compilation : Yes
Using busy-wait : Does not display among the top spots in cpu. Takes up less than 0.3%.
Handling of commandline arguments: Yes
Trace output : Yes each packet has 7 lines of trace. All the other conditions are met
Statistics output : Taken care of
Output bad format : No
Output wrong precision for statistics (should be 6-8 significant digits) : Yes
Not using condition variables or do some kind of busy-wait : Used condition variables. No busy wait
Synchronization check : Used mutex
Bad commandline : Considered

+------+
| BUGS |
+------+

Comments: none

+-------+
| OTHER |
+-------+

Comments on design decisions: as per spec
Comments on deviation from spec: no


