//--------------------------------------------------------------------------------
//
// Filename    : Deployment.h
// Description : What kind of deployment the loaded configuration describes.
//
//--------------------------------------------------------------------------------

#ifndef __DEPLOYMENT_H__
#define __DEPLOYMENT_H__

#include "KernelContext.h"
#include "Properties.h"

namespace de {

//--------------------------------------------------------------------------------
//
// conf IsNetMarble: the servers are running behind the NetMarble portal when
// the flag is set, and as their own deployment when it is zero, which is what
// every shipped configuration sets. The portal supplies the account, the
// adult verdict and the billing, so the flag decides where a login's
// verification comes from, whether the per-server user count is published,
// which system message a pay-zone portal refuses with and which dimension a
// login/logout row is written under.
//
// One reader for all of them: the flag is a single bit and reading it with
// the opposite sense at one call site silently gives that site the other
// deployment's behaviour.
//
//--------------------------------------------------------------------------------

inline bool isNetMarbleDeployment() {
    return kernelContext().config().getPropertyInt("IsNetMarble") != 0;
}

} // namespace de

#endif
