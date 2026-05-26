# stomp-messaging-server
A high-performance, asynchronous Multi-Threaded Messaging Server implementing the STOMP protocol. Features a Java-based Reactor/TPC backend architecture paired with a native, concurrent C++ client. Developed as part of the Systems Programming course at BGU.

## Development Team & Project Contributions

This repository represents a collaborative distributed systems project developed as a team of two computer science students at Ben-Gurion University. Tasks were strictly divided between core network infrastructure execution and protocol/client event architecture.

* **Sabreen Abo Saheban** (Core Server STOMP Protocol Engine, Thread-Safety, C++ Client Network Events, JSON Parsing, Integration Testing & Code Verification, Optimizing the Runtime, Data Structures & Architectural Rationale)
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

## Key Data Structures & Architectural Rationale

The performance, thread-safety, and message-routing integrity of this distributed system heavily rely on a strategic selection of data structures across both layers:

### 1. Server-Side Data Structures (Java)
* **ConcurrentHashMap Mapping Hierarchy:** To prevent race conditions in a multi-threaded network server, shared memory configurations (such as user session registries and active channel topics) are maintained via `ConcurrentHashMap`. This eliminates thread-contention via lock-striping optimization while keeping lookup times at $O(1)$.
* **Inverted Topic-to-Session Registry:** Instead of a traditional user-centric map, the server utilizes an inverted map structure: `Topic -> List<ActiveSessions>`. This design choice optimizes the message broadcast runtime, transforming an expensive double-loop lookup $O(N * M)$ into a direct, high-speed linear lookup $O(K)$ target strictly to active subscribers.

### 2. Client-Side Data Structures (C++)
* **Associative JSON Containers:** The C++ native client leverages organized associative abstractions to parse, serialize, and validate multi-field application/news events dynamically. These structured containers manage configuration key-value states efficiently before flattening them into raw compliance-ready STOMP text frames.
* **Continuous Network Byte Streams:** Employs dynamic character arrays (`std::vector<char>`) acting as linear memory buffers to stream raw network packet data continuously from sockets, ensuring reliable frame boundary isolation prior to protocol translation.

### 3. Integration Verification & State Consistency
* **State Atomicity Auditing:** As part of the integration testing lifecycle, these data structures were verified under simulated connection sequences to guarantee that protocol commands (`SUBSCRIBE` / `UNSUBSCRIBE`) cleanly trigger atomic memory updates without resulting in state corruption.
* **Static Session Lifecycle Identification:** Through rigorous integration testing, a structural design boundary was isolated: these storage maps operate on a rigid, static lifecycle scope that permanently binds the initial client connection context, highlighting the lack of a dynamic eviction/cleanup policy after a session terminates.

## System Design Constraints & Known Limitations

During the system integration and verification phase, the following architectural design constraint was isolated and documented:

* **Static Client-Server Binding (Single-User Session Limitation):** The connection handshake and session lifecycle between the C++ client and the Java server operate on a rigid, static implementation. The system is designed to accept and process a single user/client instance strictly once per execution cycle.
* **The Boundary Behavior:** If the connected user disconnects or a secondary user attempts to establish a concurrent or subsequent session, the server-client binding remains statically locked to the initial instance, requiring a manual process restart to clear the network sockets and re-initialize the state.
* **Engineering Trade-off:** This rigid behavior was kept as a project baseline constraint due to tight academic semester deadlines, shifting development focus entirely toward verifying core frame parsing correctness and basic protocol compliance rather than building a dynamically scalable multi-session lifecycle manager.
