# signup daemon

a signup daemon written in pure C code.
it must work with redis database

## it list on 1 http path: 
*  /signup

##  /signup
Accept username/sha256(real_passwd) paire submit.
For private, no recovery email/phone number is needed.
When register succeed, only has level 0 ( lowest ) piority, and waiting for administor to approved.
If approved, will raise the user-level.
If reject, will be deleted.
If administrator is too busy to deal with, it will be delected automaticlly in 30 days.
User name : length 8 - 20 chars. Only a-z/0-9 is allowed.
Password : at lease 14 chars. any visibile ASCII is allowed.

## Anti-attack:
One IP, only allow register one account per hour.
Every IP, only allow to retry 5 times, or, will automaticlly be blocked in 1 hour.



## Example html: `src/index.html`

An example how to use this signup daemon

## for more details :
Please check my [Blog](https://blog00.jjj123.com/post/2025/04/20250409_234432/).

