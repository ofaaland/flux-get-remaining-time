#! /usr/bin/env python3

import os, re
import flux
from flux.job.JobID import id_parse
from flux.job.list import get_job
import flux.uri
from flux.job import JobID, JobInfo, JobInfoFormat, JobList
from flux.util import Tree

def in_job_context(handle, jobid_str):
  try:
    jobid = id_parse(jobid_str)
  except Exception as e:
    print(f'id_parse failed with {e}')
    jobid = None
  return handle, jobid

def in_system_context(handle):
    jobid = JobID(handle.attr_get("jobid"))
    parent = flux.Flux(handle.attr_get("parent-uri"))
    return parent, jobid

def getjobid(handle):
  jobid_str = os.environ.get('FLUX_JOB_ID')
  if jobid_str is None:
    return in_system_context(handle)
  else:
    return in_job_context(handle, jobid_str)

def get_scr_end_time(handle, jobid):
  job = get_job(handle, jobid)
  if job is not None:
    expiration = job['expiration']
  else:
    print(f'unable to extract expiration from job {job}')
    expiration = 0
  return expiration

handle = flux.Flux()
print(f'flux is {flux}')

handle, jobid = getjobid(handle)
print(f'flux jobid: {jobid}')

end_time = get_scr_end_time(handle, jobid)
print(f'flux end_time: {end_time}')
