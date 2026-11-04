#ifndef DARKEDEN_LOGIN_KICK_RETRY_H
#define DARKEDEN_LOGIN_KICK_RETRY_H

#include <functional>
#include <string>

#include "Timeval.h"

class LoginPlayer;

namespace de {

// A successful request publishes its waiting phase and next deadline. False
// means refusal; the action owns its response and session changes. Exceptions
// propagate, including after a partial broadcast. Callers serialize player access.
using LoginKickRequest = std::function<bool(LoginPlayer&)>;

// An authenticated fresh attempt resets the retry count before requesting its
// first kick, including when that request refuses or throws. Cached routing and
// the previous deadline remain intact until the request changes them.
bool beginLoginKick(LoginPlayer& player, const LoginKickRequest& send);

// At or after the deadline, resend only for a waiting, account-bound target.
// Count a successful resend only while the same account, target and local
// attempt remain pending. Complete after the third retry (four sends in all).
// The count saturates at three; if completion throws without ending the wait,
// the next deadline permits another resend/completion. Actions own mutations;
// neither refusal nor an exception is rolled back. True means a retry was
// counted and any required completion returned normally.
bool retryLoginKick(LoginPlayer& player, const Timeval& now, const LoginKickRequest& send,
                    const std::function<void(LoginPlayer&)>& complete);

// What a kick-wait failure is reported as: the exception's kind alone, never
// its message, which can carry account data.
using LoginKickFailureReport = std::function<void(const std::string& kind)>;

// The kick-wait step of LoginPlayer::processCommand: retryLoginKick, with the
// resend's and the completion's failures decided as the dispatch wrap decides
// a handler's. A ProtocolException and a ConnectException pass on as they
// are, for the manager to retire the player; anything else they throw, a
// DatabaseError from the completion's account statements among them, is
// reported through `failed` by its kind and becomes a DisconnectException,
// so the manager retires the player as for a logout instead of the throw
// reaching the main loop. The actions' effects stand as retryLoginKick
// leaves them; a report that throws changes nothing.
bool runLoginKickWait(LoginPlayer& player, const Timeval& now, const LoginKickRequest& send,
                      const std::function<void(LoginPlayer&)>& complete, const LoginKickFailureReport& failed);

} // namespace de

#endif
