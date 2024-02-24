# circb

## Description

circb is an IRC (Internet Relay Chat) network written in C, with support for arbitrarily many servers, channels, and users, and following [the IRC protocol (RFC 1459)](https://www.rfc-editor.org/rfc/rfc1459.html).

For a more robust chat app and pleasant user experience (including UI), an app built on top of circb is in development (but will link here when it is live).

## Dev TODOs

* implement graceful server shutdown
* define strategy and be ready to implement for handling of errors, management of state, I/O, and concurrency
* implement first "basic" IRC commands requiring only server/client (maybe NICK, USER)
* create bare bones channel component
* implement second set of "basic" commands (e.g. JOIN, PART); test functioning channel using a single server with multiple users joined
* implement server linking, enable message propogation btw servers, start testing with multiple servers
* start `docs/`; begin `circb` documentation
* write unit tests for first commands
* implement testing framework (from scratch, start small)
* finish implementing the set of "basic" commands, including testing
* set up foundation and start documentation for organized unit, integration, and end-to-end testing
* implement additional sub-component types such as operators and channel operators
* implement user auth
* implement and document security protocol
* perform stress testing on "basic" setup; simulate introduction of bad actors
* implement additional commands and features outside the scope of the original IRC (what is value added by circb specifically?); continue adding testing and documentation

## Features

* User authentication
* Secure communication
* Scalability
* Fault tolerance
* Unit, integration, and end-to-end testing

## Project Structure

```text
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
  * The commands which may only be used by channel operators are:

    ```text
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

```text
    nodeID       -
    hostname     -
    local-port   -
    routing-port -
    IRC-port     -
    TKTK         -
```

(optional)

```text
    TKTK -
    TKTK -
```

**Sample Configuration File:**

```text
// node1.conf
TKTK
TKTK
TKTK
```

## Interaction Protocols

For now, this project follows the IRC protocol more or less, though specific component interactions and commands may differ. For full details of circb's available commands and protocol please see the [circb Interaction Protocol Google Sheet](https://docs.google.com/spreadsheets/d/1ZwGiwEt0Bo0nahpE2eTKUP_niFaKW3wwh9WVr5oVtIc).

For example,

```text
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

## Potential Future Additions

* OPTIONAL commands (i.e. optional from IRC protocol perspective, aka QOL additions)
  * TKTK
* Features
  * Smart handling of race conditions
  * Flood control of clients
