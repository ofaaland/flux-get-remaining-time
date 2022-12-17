#! /usr/bin/env python3

import os, re
import flux
from flux.job.JobID import id_parse
from flux.job.list import get_job

def getjobid(handle):
  if True:
    jobid_str = os.environ.get('FLUX_JOB_ID')
    if jobid_str is None:
      jobid_str = handle.attr_get('jobid')

    try:
      jobid = id_parse(jobid_str)
    except Exception as e:
      print(f'id_parse failed with {e}')
      jobid = None

    return handle, jobid

  return None

def get_scr_end_time(handle, jobid):
  job = get_job(handle, jobid)
  if job is not None:
    expiration = job['expiration']
  else:
    print(f'unable to extract expiration from job {job}')
    expiration = 0
  return expiration

flux = flux.Flux()
print(f'flux is {flux}')

handle, jobid = getjobid(flux)
print(f'flux jobid: {jobid}')

end_time = get_scr_end_time(handle, jobid)
print(f'flux end_time: {end_time}')
