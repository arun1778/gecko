/* ----- BEGIN LICENSE BLOCK -----
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape Communications Corporation.
 * Portions created by Netscape Communications Corporation are
 * Copyright (C) 2001 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the LGPL or the GPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ----- END LICENSE BLOCK ----- */

#include "inSearchLoop.h"

#include "nsITimer.h"

///////////////////////////////////////////////////////////////////////////////

inSearchLoop::inSearchLoop(inISearchProcess* aSearchProcess)
{
  mSearchProcess = aSearchProcess;
  nsresult rv;
  mTimer = do_CreateInstance("@mozilla.org/timer;1", &rv);
}

inSearchLoop::~inSearchLoop()
{
}

///////////////////////////////////////////////////////////////////////////////
// inSearchLoop

nsresult
inSearchLoop::Start()
{
  mTimer->Init(inSearchLoop::TimerCallback, (void*)this, 0, NS_PRIORITY_NORMAL, NS_TYPE_REPEATING_SLACK);

  return NS_OK;
}

nsresult
inSearchLoop::Step()
{
  PRBool done = PR_FALSE;
  mSearchProcess->SearchStep(&done);

  if (done)
    Stop();

  return NS_OK;
}

nsresult
inSearchLoop::Stop()
{
  mTimer->Cancel();
  
  return NS_OK;
}

void 
inSearchLoop::TimerCallback(nsITimer *aTimer, void *aClosure)
{
  inSearchLoop* loop = (inSearchLoop*) aClosure;
  loop->Step();
}
