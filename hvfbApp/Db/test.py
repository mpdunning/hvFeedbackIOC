#!/usr/bin/env python

import random
from time import sleep
from epics import PV

pv1 = PV('ESB:GP01:VAL03')
random.seed()

while True:
    rand1 = random.uniform(-5, 5)
    #print rand1
    val0 = pv1.get()
    newval = val0 + rand1
    print newval
    pv1.put(newval)
    sleep(20)


