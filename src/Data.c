/********************************************************************\
 * Data.c -- the main data structure of the program                 *
 * Copyright (C) 1997 Robin D. Clark                                *
 * Copyright (C) 1997 Linas Vepstas                                 *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, write to the Free Software      *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.        *
 *                                                                  *
 *   Author: Rob Clark                                              *
 * Internet: rclark@cs.hmc.edu                                      *
 *  Address: 609 8th Street                                         *
 *           Huntington Beach, CA 92648-4632                        *
\********************************************************************/

#include "Account.h"
#include "Data.h"
#include "util.h"

/********************************************************************\
 * Because I can't use C++ for this project, doesn't mean that I    *
 * can't pretend too!  These functions perform actions on the       *
 * AccountGroup data structure, in order to encapsulate the         *
 * knowledge of the internals of the AccountGroup in one file.      *
\********************************************************************/

AccountGroup *topgroup = 0x0;

/********************************************************************\
\********************************************************************/
void
xaccInitializeAccountGroup (AccountGroup *grp)
  {
  grp->saved       = True;
  grp->new         = False;
  
  grp->parent      = NULL;
  grp->numAcc      = 0;
  grp->account     = NULL;
  
  grp->balance     = 0.0;
  }

/********************************************************************\
\********************************************************************/
AccountGroup *
mallocAccountGroup( void )
  {
  AccountGroup *grp = (AccountGroup *)_malloc(sizeof(AccountGroup));
  
  xaccInitializeAccountGroup (grp);

  return grp;
  }

/********************************************************************\
\********************************************************************/
void
freeAccountGroup( AccountGroup *grp )
  {
  if( grp != NULL )
    {
    int i;

    for( i=0; i<grp->numAcc; i++ )
      freeAccount( grp->account[i] );
    
    _free( grp->account );

    /* null everything out, just in case somebody 
     * tries to traverse freed memory */
    xaccInitializeAccountGroup (grp);

    _free(grp);
    }
  }

/********************************************************************\
\********************************************************************/
void
xaccAccountGroupMarkSaved (AccountGroup *grp)
{
   int i;

   grp->saved = True;

   for (i=0; i<grp->numAcc; i++) {
      xaccAccountGroupMarkSaved (grp->account[i]->children); 
   }
}

/********************************************************************\
\********************************************************************/
int
xaccAccountGroupNotSaved (AccountGroup *grp)
{
   int not_saved;
   int i;

   if (False == grp->saved) return 1;

   for (i=0; i<grp->numAcc; i++) {
      not_saved = xaccAccountGroupNotSaved (grp->account[i]->children); 
      if (not_saved) return 1;
   }
   return 0;
}

/********************************************************************\
\********************************************************************/
Account *
getAccount( AccountGroup *grp, int num )
  {
  if( grp != NULL )
    {
    if( (0 <= num) && (num < grp->numAcc) )
      return grp->account[num];
    else
      return NULL;
    }
  else
    return NULL;
  }

/********************************************************************\
 * Get the number of accounts, including subaccounts                *
\********************************************************************/

int
xaccGetNumAccounts ( AccountGroup *root )
{
  Account *acc;
  int num_acc = 0;
  int i;

  if (NULL == root) return 0;

  num_acc = root->numAcc;

  for (i=0; i<root->numAcc; i++) {
    num_acc += xaccGetNumAccounts (root->account[i]->children);
  }

  return num_acc;
}

/********************************************************************\
 * Fetch an account, given only it's ID number                      *
\********************************************************************/

Account *
xaccGetAccountFromID ( AccountGroup *root, int acc_id )
{
  Account *acc;
  int i;

  if (NULL == root) return NULL;
  if (-1 >= acc_id) return NULL;

  /* first, look for accounts hanging off the root */
  for (i=0; i<root->numAcc; i++) {
    acc = root->account[i];
    if (acc_id == acc->id) return acc;
  }

  /* if we are still here, then we haven't found the account yet.
   * Recursively search the subgroups next */
  for (i=0; i<root->numAcc; i++) {
    acc = xaccGetAccountFromID (root->account[i]->children, acc_id);
    if (acc) return acc;
  }

  return NULL;
}

/********************************************************************\
 * Fetch the root of the tree                                       *
\********************************************************************/

AccountGroup *
xaccGetAccountRoot (Account * acc) 
{
  Account *parent_acc;
  AccountGroup * grp;
  AccountGroup * root;

  /* find the root of the account group structure */
  grp = (AccountGroup *) acc->parent;
  while (grp) {
    root = grp;
    parent_acc = grp -> parent;
    grp = NULL;
    if (parent_acc) {
       grp = (AccountGroup *) parent_acc->parent;
    }
  }
  return root;
}

/********************************************************************\
 * Fetch an account, given only it's ID number                      *
\********************************************************************/

Account *
xaccGetPeerAccountFromID ( Account *acc, int acc_id )
{
  AccountGroup * root;
  Account *peer_acc;
  int i;

  if (NULL == acc) return NULL;
  if (-1 >= acc_id) return NULL;

  /* first, find the root of the account group structure */
  root = xaccGetAccountRoot (acc);

  /* now search all acounts hanging off the root */
  peer_acc = xaccGetAccountFromID (root, acc_id);

  return peer_acc;
}

/********************************************************************\
 * Fetch an account, given it's name                                *
\********************************************************************/

Account *
xaccGetAccountFromName ( AccountGroup *root, char * name )
{
  Account *acc;
  int i;

  if (NULL == root) return NULL;
  if (NULL == name) return NULL;

  /* first, look for accounts hanging off the root */
  for (i=0; i<root->numAcc; i++) {
    acc = root->account[i];
    if (!strcmp(acc->accountName, name)) return acc;
  }

  /* if we are still here, then we haven't found the account yet.
   * Recursively search the subgroups next */
  for (i=0; i<root->numAcc; i++) {
    acc = xaccGetAccountFromName (root->account[i]->children, name);
    if (acc) return acc;
  }

  return NULL;
}

/********************************************************************\
 * Fetch an account, given it's name                                *
\********************************************************************/

Account *
xaccGetPeerAccountFromName ( Account *acc, char * name )
{
  AccountGroup * root;
  Account *peer_acc;
  int i;

  if (NULL == acc) return NULL;
  if (NULL == name) return NULL;

  /* first, find the root of the account group structure */
  root = xaccGetAccountRoot (acc);

  /* now search all acounts hanging off the root */
  peer_acc = xaccGetAccountFromName (root, name);

  return peer_acc;
}

/********************************************************************\
\********************************************************************/
Account *
removeAccount( AccountGroup *grp, int num )
  {
  Account *acc = NULL;
  
  if( NULL != grp )
    {
    int i,j;
    Account **oldAcc = grp->account;

    grp->saved = False;
    
    grp->numAcc--;
    grp->account = (Account **)_malloc((grp->numAcc)*sizeof(Account *));
    
    acc = oldAcc[grp->numAcc];    /* In case we are deleting last in
				    * old array */
    for( i=0,j=0; i<grp->numAcc; i++,j++ )
      {
      if( j != num )
        grp->account[i] = oldAcc[j];
      else
        {
        acc = oldAcc[j];
        j--;
        }
      }
    
    _free(oldAcc);
    }
  return acc;
  }

/********************************************************************\
\********************************************************************/

void
xaccRemoveAccount (Account *acc)
{
   int i,j;
   AccountGroup *grp;
   Account **oldAcc;

   if (NULL == acc) return;
   grp = acc->parent;

   oldAcc = grp->account;

   grp->saved = False;
    
   grp->numAcc--;
   grp->account = (Account **)_malloc((grp->numAcc)*sizeof(Account *));
    

   for( i=0,j=0; i<grp->numAcc; i++,j++ ) {
      if( acc != grp->account[i] ) {
         grp->account[i] = oldAcc[j];
      } else {
         j--;
      }
   }
    
   _free(oldAcc);

   freeAccount (acc);
}


/********************************************************************\
\********************************************************************/
int
xaccInsertSubAccount( Account *parent, Account *child )
{
  int retval;

  if (NULL == parent) return -1;
  if (NULL == child) return -1;

  /* if a container for the children doesn't yet exist, add it */
  if (NULL == parent->children) {
    parent->children = mallocAccountGroup();
  }

  retval = insertAccount (parent->children, child);
  return retval;
}

/********************************************************************\
\********************************************************************/
int
insertAccount( AccountGroup *grp, Account *acc )
  {
  int i=-1;
  Account **oldAcc;
  
  if (NULL == grp) return -1;
  if (NULL == acc) return -1;

  /* set back-pointer to the accounts parent */
  acc->parent = (struct _account_group *) grp;

  oldAcc = grp->account;
    
  grp->saved = False;
  
  grp->numAcc++;
  grp->account = (Account **)_malloc((grp->numAcc)*sizeof(Account *));
  
  for( i=0; i<(grp->numAcc-1); i++ )
    grp->account[i] = oldAcc[i];
  
  grp->account[i] = acc;
  
  _free(oldAcc);

  return i;
  }


/****************** END OF FILE *************************************/
