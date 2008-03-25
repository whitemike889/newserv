/* Automatically generated by refactor.pl.
 *
 *
 * CMDNAME: cleanupdb
 * CMDLEVEL: QCMD_DEV
 * CMDARGS: 0
 * CMDDESC: Clean up database.
 * CMDFUNC: csu_docleanupdb
 * CMDPROTO: int csu_docleanupdb(void *source, int cargc, char **cargv);
 * CMDHELP: Usage: cleanupdb
 * CMDHELP: Cleans up inactive accounts, unused accounts and inactive channels.
 */

#include "../chanserv.h"
#include "../../lib/irc_string.h"
#include <stdio.h>
#include <string.h>

int csu_docleanupdb(void *source, int cargc, char **cargv) {
  nick *sender=source;
  reguser *vrup, *srup, *founder;
  authname *anp;
  int i;
  time_t t;
  long to_age, unused_age, maxchan_age;
  int expired = 0, unauthed = 0, chansvaped = 0;
  chanindex *cip, *ncip;
  regchan *rcp;

  t = time(NULL);
  to_age = t - (CLEANUP_ACCOUNT_INACTIVE * 3600 * 24);  
  unused_age = t - (CLEANUP_ACCOUNT_UNUSED * 3600 * 24);
  maxchan_age = t - (CLEANUP_CHANNEL_INACTIVE * 3600 * 24);

  cs_log(sender, "CLEANUPDB started");

  chanservsendmessage(sender, "Scanning regusers...");
  for (i=0;i<REGUSERHASHSIZE;i++) {
    for (vrup=regusernicktable[i]; vrup; vrup=srup) {
      srup=vrup->nextbyname;
      if (!(anp=findauthname(vrup->ID)))
        continue; /* should maybe raise hell instead */

      if(!anp->nicks && !UHasHelperPriv(vrup) && !UIsCleanupExempt(vrup)) {
        if(vrup->lastauth && (vrup->lastauth < to_age)) {
          expired++;
          cs_log(sender, "CLEANUPDB inactive user %s", vrup->username);
        } else if(!vrup->lastauth && (vrup->created < unused_age)) {
          unauthed++;
          cs_log(sender, "CLEANUPDB unused user %s", vrup->username);
        } else {
          continue;
        }

        cs_removeuser(vrup);
      }
    }
  }

  chanservsendmessage(sender, "Scanning chanindicies...");
  for (i=0;i<CHANNELHASHSIZE;i++) {
    for (cip=chantable[i];cip;cip=ncip) {
      ncip=cip->next;
      if (!(rcp=cip->exts[chanservext]))
        continue;

      if(rcp->lastactive < maxchan_age) {
        /* don't remove channels with the original founder as an oper */
        founder=findreguserbyID(rcp->founder);
        if(founder && UHasOperPriv(founder))
          continue;

        cs_log(sender, "CLEANUPDB inactive channel %s", cip->name?cip->name->content:"??");
        cs_removechannel(rcp);
        chansvaped++;
      }
      
      /* Get rid of any dead chanlev entries */
      for (j=0;j<REGCHANUSERHASHSIZE;j++) {
        for (rcup=rcp->users[j];rcup;rcup=nrcup} {
          nrcup=rcup->nextbychan;
          
          if (!rcup->flags) {
            chanservsendmessage("Removing user %s from channel %s (no flags)",rcup->user->username,rcp->index->name->content);
/*
            csdb_deletechanuser(rcup);
            delreguserfromchannel(rcp, rcup->user);
*/
          }
        }
      }
    }
  }
  
  cs_log(sender, "CLEANUPDB complete %d inactive accounts %d unused accounts %d channels", expired, unauthed, chansvaped);
  chanservsendmessage(sender, "Cleanup complete, %d accounts inactive for %d days, %d accounts weren't used within %d days, %d channels were inactive for %d days.", expired, CLEANUP_ACCOUNT_INACTIVE, unauthed, CLEANUP_ACCOUNT_UNUSED, chansvaped, CLEANUP_CHANNEL_INACTIVE);
  return CMD_OK;
}
