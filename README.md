# stomp-messaging-server
A high-performance, asynchronous Multi-Threaded Messaging Server implementing the STOMP protocol. Features a Java-based Reactor/TPC backend architecture paired with a native, concurrent C++ client. Developed as part of the Systems Programming course at BGU.

## Development Team & Project Contributions

This repository represents a collaborative distributed systems project developed as a team of two software engineering/computer science students at Ben-Gurion University. Tasks were strictly divided between core network infrastructure execution and protocol/client event architecture.

* **Sabreen Abo Saheban** (Core Server STOMP Protocol Engine, C++ Client Network Events, JSON Parsing, Integration Testing & Code Verification)
* **[Rasan Abo Kaf]** (Server Concurrency Layer, Thread-Safety, & Locking Mechanics)

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
