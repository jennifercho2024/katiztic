/* owners.h — the cat owners you meet walking their cats on the street.
 *
 * Greet an owner and pet each other's cats a few times and they warm to you;
 * once you're friends they'll send a letter inviting your cats on a playdate.
 * This module remembers each owner: how friendly you are, whether you've become
 * friends, their cat's color, and whether a playdate invitation is waiting in
 * your mailbox.
 */
#ifndef KATIZTIC_OWNERS_H
#define KATIZTIC_OWNERS_H

#include <SDL3/SDL.h>
#include "cattype.h"

#define KZ_OWNER_NAME   16
#define KZ_MAX_OWNERS   8
#define OWNER_GREETS_TO_FRIEND 3   /* friendly greetings needed to befriend */

typedef struct {
    char    name[KZ_OWNER_NAME];
    CatType cat_type;      /* the color of their cat            */
    Uint8   greets;        /* how many times you've stopped to say hi */
    bool    befriended;    /* are you friends yet?             */
    bool    invite_pending;/* is a playdate letter waiting?    */
    bool    invite_read;   /* have you opened that letter?     */
} Owner;

typedef struct {
    Owner list[KZ_MAX_OWNERS];
    int   count;
} Owners;

Owners owners_new(void);

/* Find an owner by name, or NULL. */
Owner *owners_find(Owners *o, const char *name);

/* Record greeting/petting an owner's cat. Adds them if new. Returns:
 *   1  if this greeting just made you friends (they'll send an invite)
 *   0  otherwise
 * The owner's cat type is set on first meeting. */
int owners_greet(Owners *o, const char *name, CatType cat_type);

/* How many friends you've made. */
int owners_friend_count(const Owners *o);

/* How many playdate invitations are waiting (pending, whether read or not). */
int owners_invite_count(const Owners *o);

/* Clear a fulfilled invite once a playdate is done. */
void owners_clear_invite(Owners *o, const char *name);

bool owners_save(const Owners *o, const char *path);
bool owners_load(Owners *o, const char *path);

#endif /* KATIZTIC_OWNERS_H */