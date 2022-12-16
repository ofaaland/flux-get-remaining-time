/***************************************************************************
 *  Copyright (C) 2017, Lawrence Livermore National Security, LLC.
 *  Produced at the Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Olaf Faaland <faaland1@llnl.gov>
 *  UCRL-CODE-235649. All rights reserved.
 *
 *  This file is part of libyogrt.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library.  If not, see
 *  <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <flux/core.h>

#define BOGUS_TIME -1

int jobid_valid = 0;
int verbosity = 0;

static flux_jobid_t jobid = 0;

void
error (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
}

void
debug (const char *fmt, ...)
{
    va_list ap;

    if (verbosity > 0) {
        va_start (ap, fmt);
        vfprintf (stderr, fmt, ap);
        va_end (ap);
    }
}

void die(char *msg)
{
	error("ERROR: %s\n", msg);
	exit(1);
}

int internal_init(int verb)
{
	const char *jobid_str;
	flux_t *h = NULL;

	verbosity = verb;
	jobid_valid = 0;

	if ((jobid_str = getenv("FLUX_JOB_ID")) == NULL) {
		if (!(h = flux_open(NULL, 0))) {
			error("ERROR: flux_open() failed with errno %d\n", errno);
			return jobid_valid;
		}

		jobid_str = (const char *) flux_attr_get(h, "jobid");
		if (jobid_str == NULL) {
			error("ERROR: Unable to obtain flux job ID, errno %d.\n");
			return jobid_valid;
		}
	}

	if (flux_job_id_parse(jobid_str, &jobid) < 0) {
		error("ERROR: Unable to parse FLUX_JOB_ID %s."
			  " Remaining time will be a bogus value.\n", jobid_str);
		return jobid_valid;
	}

	jobid_valid = 1;

	flux_close(h);
	return jobid_valid;
}

static int get_job_expiration(flux_jobid_t id, long int *expiration, int *source)
{
	flux_t *h = NULL;
	flux_t *child_handle = NULL;
	flux_future_t *f;
	double exp;
	const char *uri = NULL;
	int rc = -1;

	if (!(h = flux_open(NULL, 0))) {
		error("ERROR: flux_open() failed with errno %d\n", errno);
		goto out;
	}

	/*
	 * Determine whether to ask our parent or not
	 * See https://github.com/flux-framework/flux-core/issues/3817
	 */

	if (!getenv("FLUX_KVS_NAMESPACE")) {
		uri = flux_attr_get(h, "parent-uri");
		if (!uri) {
			error("ERROR: no FLUX_KVS_NAMESPACE and flux_attr_get failed with "
				  "errno %d\n", errno);
			goto out;
		}

		child_handle = h;
		h = flux_open(uri, 0);
		if (!h) {
			printf("flux_open with parent-uri %s failed with errno %d\n", uri,
				   errno);
			goto out;
		}

		debug("No FLUX_KVS_NAMESPACE, querying parent at %s\n", uri);
		*source = 1;
	} else {
		*source = 0;
	}

	if (!(f = flux_job_list_id(h, jobid, "[\"expiration\"]"))) {
		error("ERROR: flux_job_list failed with errno %d.\n", errno);
		*source = -1;
		goto out;
	}

	if (flux_rpc_get_unpack (f, "{s:{s:f}}", "job", "expiration", &exp) < 0) {
		error("ERROR: flux_rpc_get_unpack failed with errno %d.\n", errno);
		*source = -1;
		goto out;
	}

	*expiration = (long int) exp;
	rc = 0;

out:
	flux_future_destroy(f);
	flux_close(h);
	flux_close(child_handle);

	return rc;
}

int internal_get_rem_time(time_t now)
{
	long int expiration;
	int remaining_sec = BOGUS_TIME;
	int from_parent = -1;
	char *source_strings[3] = {"parent", "child", "unknown"};
	char *source = NULL;

	if (! jobid_valid) {
		error("FLUX: No valid jobid to lookup!\n");
		return BOGUS_TIME;
	}

	if (get_job_expiration(jobid, &expiration, &from_parent)) {
		error("FLUX: get_job_expiration failed\n");
		goto out;
	}

	switch (from_parent) {
		case 1:
			source = source_strings[0];
			break;
		case 0:
			source = source_strings[1];
			break;
		case -1:
			source = source_strings[2];
			break;
	}

	remaining_sec = (int) (expiration - time(NULL));
	debug("flux remaining seconds (from %s) is %ld sec (%.2f hrs)\n", source, remaining_sec, (double) remaining_sec / 3600);
	printf("flux remaining seconds (from %s) is %ld sec (%.2f hrs)\n", source, remaining_sec, (double) remaining_sec / 3600);

out:
	return remaining_sec;
}

int internal_get_rank(void)
{
	char *rank_str;

	rank_str = getenv("FLUX_TASK_RANK");

	if (rank_str) {
		return atoi(rank_str);
	} else {
		return 0;
	}
}

int main(int argc, char **argv)
{
	int rc, remaining;
	
	rc = internal_init(0);
	if (!rc)
		die("internal_init failed");

	remaining = internal_get_rem_time(time(NULL));
}

/*
 * vim: tabstop=8 shiftwidth=8 smartindent:
 */
