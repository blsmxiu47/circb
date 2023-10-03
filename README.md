# circb: An IRC (Internet Relay Chat) system, written in C

circb (or "C IRC blsmxiu", because clearly I'm great at naming things) is (will be) an IRC network, with support for many servers, channels, and users, and following [the IRC protocol (RFC 1459)](https://www.rfc-editor.org/rfc/rfc1459.html).

For a more robust chat app and pleasant user experience (including UI), please see [TKTK](have not developed this yet), which is a product I will develop in ??Go, ??Rust, ??Zig, not sure yet; guessing my prefs will change over time.

## While in development

*Some things to note:*
* When looking at the commit history for this repo there may be a lot of backtracking and trail and error over time. I enjoy learning (shamelessly) as I gradually make progress toward some eventual product that no one will ever use. Most likely some components will be developed only to be removed later entirely; all part of the process though ^^
* Will clean up the finished product if I ever get there, but until then I may chat with myself in comments, this README, etc. All due apologies.

## Features

* TKTK

## WIP structure considerations

```
├── src
│   ├── components
│   │   ├── *.c
│   ├── handlers 
│   ├── utils
├── include
│   │   ├── *.h
├── tests
```

## Components and Interactions

* Server (server.c)
* Client (user.c, operator.c, chop.c)
  *   The commands which may only be used by channel operators are:
        ```
        KICK    - Eject a client from the channel
        MODE    - Change the channel's mode
        INVITE  - Invite a client to an invite-only channel (mode +i)
        TOPIC   - Change the channel topic in a mode +t channel
        ```
* Channel

## Configuration File

The configuration file required for starting an instance of circb has some requirements that may differ from those of other popular IRCs.

**Configuration File Parameters**
(required)
```
    nodeID       -
    hostname     -
    local-port   -
    routing-port -
    IRC-port     -
    TKTK         -
```

(optional)
```
    TKTK -
    TKTK -
```

**Sample Configuration File**
```
// node1.conf
TKTK
TKTK
TKTK
```



## Interaction Protocols

For now, this project follows the IRC protocol more or less, though specific component interactions and commands may differ. For full details of circb's available commands and protocol please see the [circb Interaction Protocol Google Sheet](https://docs.google.com/spreadsheets/d/1ZwGiwEt0Bo0nahpE2eTKUP_niFaKW3wwh9WVr5oVtIc).

For example,
```
    Command: JOIN
    Format: JOIN <#channel>
    Sender: Client/User
    Receiver: Server
    Actions:
        Server:
            * Validate the command format.
            * Check if the channel exists.
            * Add the user to the channel's user list.
            * Send acknowledgment to the user.
            * Notify other users in the channel.
        Client/User:
            * Send the properly formatted command to the server.
            * Wait for acknowledgment.
            * Update local state upon acknowledgment.
    Error Handling:
        * If the channel doesn’t exist, return an error to the sender.
        * If the command format is invalid, return an error to the sender.
```

## In addition I hope to add........

The following functionality. But have not yet gotten to it.
* The following OPTIONAL commands (i.e. optional from IRC protocol perspective, aka QOL additions)
  * A
  * B
  * C
  * etc...
* Features of note(??)
  * Smart handling of race conditions
  * Flood control of clients
  * emdemd

## Getting Started

TKTK
