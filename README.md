# stomp-messaging-server
A high-performance, asynchronous Multi-Threaded Messaging Server implementing the STOMP protocol. Features a Java-based Reactor/TPC backend architecture paired with a native, concurrent C++ client. Developed as part of the Systems Programming course at BGU.

## Development Team & Project Contributions

This repository represents a collaborative distributed systems project developed as a team of two software engineering/computer science students at Ben-Gurion University. Tasks were strictly divided between core network infrastructure execution and protocol/client event architecture.

* **Sabreen Abo Saheban** (Core Server STOMP Protocol Engine, C++ Client Network Events, JSON Parsing, Integration Testing & Code Verification)
* **My team memeber** (Server Concurrency Layer, Thread-Safety, & Locking Mechanics)

### Detailed Breakdown of My Personal Work (Sabreen Abo Saheban)

#### 1. Server-Side Protocol Engine (Built over the provided Network Skeleton)
* **STOMP Protocol Parsing & State Machine:** Engineered the stateful messaging protocol translation layer on the server side. Programmed the logic that deserializes raw incoming network packets into structured, validated STOMP frames (CONNECT, SUBSCRIBE, SEND, DISCONNECT) to trigger backend actions.
* **Server-Side Encoding:** Developed the frame serialization layer to ensure outgoing messages adhere strictly to the network protocol specifications before transmission over active sockets.

#### 2. Native C++ Concurrent Client
* **Asynchronous Network Event Cycle:** Implemented the client-side continuous event loop, enabling the native C++ client to listen to network sockets and process server frames concurrently without freezing or blocking user interface interactions.
* **JSON Serialization & Parsing:** Integrated and developed the data parsing sub-layer using C++ JSON structures to dynamically read complex application/news events and translate them into compliant, sequential STOMP frames.

#### 3. Integration, Verification & System-Wide Testing
* **Cross-Layer Integration Testing:** Acted as the primary integration tester for the project. Merged the concurrent server execution layer (developed by the team member) with the protocol engine and the native C++ client to ensure flawless end-to-end subsystem communication.
* **Code Verification & Debugging:** Conducted rigorous testing and manual code reviews on the shared state mechanics to verify that the locking primitives and thread-safe collections behaved properly under high-concurrency simulation loads, isolating protocol bugs before final deployment.

## System Design Constraints & Known Limitations

During the system integration and verification phase, the following architectural design constraint was isolated and documented:

* **Static Client-Server Binding (Single-User Session Limitation):** The connection handshake and session lifecycle between the C++ client and the Java server operate on a rigid, static implementation. The system is designed to accept and process a single user/client instance strictly once per execution cycle.
* **The Boundary Behavior:** If the connected user disconnects or a secondary user attempts to establish a concurrent or subsequent session, the server-client binding remains statically locked to the initial instance, requiring a manual process restart to clear the network sockets and re-initialize the state.
* **Engineering Trade-off:** This rigid behavior was kept as a project baseline constraint due to tight academic semester deadlines, shifting development focus entirely toward verifying core frame parsing correctness and basic protocol compliance rather than building a dynamically scalable multi-session lifecycle manager.
