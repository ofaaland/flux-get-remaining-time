#! /usr/bin/env python3

import flux

# from flux-pstree.py
import flux.uri
from flux.job import JobID, JobInfo, JobInfoFormat, JobList
from flux.util import Tree

#handle = flux.Flux()
##size = handle.attr_get("size")
#jobid = JobID(handle.attr_get("jobid"))
#parent = flux.Flux(handle.attr_get("parent-uri"))
#info = JobList(parent, ids=[jobid]).fetch_jobs().get_jobs()[0]
#print(f'parent is {parent}')
#print(f'info is {info}')

# my code
handle = flux.Flux()
print(f'handle is {handle}')
parenturi = handle.attr_get('parent-uri')
print(f'parenturi is {parenturi}')
parent = handle.Flux(parenturi)
