# STOMP Messaging Server & Native C++ Client

A high-performance, asynchronous, multi-threaded messaging server implementing the **STOMP protocol**. Originally developed as a team project for the Systems Programming course at Ben-Gurion University (BGU), this project was later independently re-engineered and optimized to achieve production-grade stability.

## 🚀 Project Overview
This distributed system facilitates real-time communication between multiple clients and a central server. The architecture emphasizes thread safety, low-latency message routing, and robust socket lifecycle management. 

> **Note:** Although originally a team project, this version reflects significant independent post-course re-engineering and architectural stabilization performed solely by **Sabreen Abo Saheban**.

## 🛠 Technical Contributions
* **Server-Side Protocol Engine:** Engineered the stateful protocol translation layer to deserialize raw network packets into validated STOMP frames (`CONNECT`, `SUBSCRIBE`, `SEND`, `DISCONNECT`).
* **Native C++ Concurrent Client:** Implemented a non-blocking asynchronous event loop, enabling simultaneous socket listening and interactive CLI handling.
* **JSON Serialization:** Developed a custom parsing layer to translate complex application events into compliant STOMP frames.
* **System Stability:** Conducted rigorous integration testing, auditing locking primitives and thread-safe collections to prevent race conditions.

## 🏗 Architectural Rationale
### Server-Side (Java)
* **`ConcurrentHashMap` Hierarchy:** Manages session and topic registries with $O(1)$ lookup complexity, optimized for high concurrency.
* **Inverted Topic-to-Session Mapping:** Optimized broadcast performance, reducing lookup complexity from $O(N * M)$ to $O(K)$.

### Client-Side (C++)
* **Buffered Stream Management:** Leverages dynamic memory buffers for efficient streaming and packet boundary isolation.
* **Resilient Connection Logic:** Implemented an incremental backoff algorithm to manage session transitions.

---

## ⚙️ Engineering Challenges & Resolution: Achieving Systemic Stability

Four months after the project's academic completion, I returned to the codebase to address a critical architectural bottleneck: **Intermittent `Broken pipe` errors** occurring during rapid `Login -> Logout -> Login` cycles.

### Root Cause Analysis
The issue was identified as a dual-layered architectural struggle:
1.  **Socket Teardown Latency:** A race condition where the client initiated a new connection before the server finalized the teardown of the previous session.
2.  **Resource Leakage:** Insufficient explicit cleanup of `ConnectionHandler` instances and memory buffers during rapid re-login cycles, leading to state corruption.

### The "Deep Dive" Refinement (Post-Course Independent Work)
I conducted a comprehensive system overhaul to achieve systemic stability:
* **Deterministic Memory & Lifecycle Cleanup:** I re-engineered the client-side termination logic to ensure that memory buffers are explicitly cleared and `ConnectionHandler` instances are safely evicted from memory before a new session is initialized.
* **Robust Reconnection Algorithm:** I integrated an incremental backoff mechanism (Exponential Delay) on the client side. This provides the server the necessary "breathing room" to finalize socket cleanup and registry eviction.

### Result
This independent, dual-layered refinement transformed the project into a robust, production-grade engine, achieving **100% stability** in session cycling and guaranteeing memory consistency across rapid login/logout sequences.

---

## 📝 Features & Protocol Compliance
* **Full Protocol Handshake:** Seamless execution of `CONNECT`, `SUBSCRIBE`, `SEND`, and `DISCONNECT` flows.
* **Dynamic Event Injection:** Efficient JSON parsing and distribution.
* **Strict Isolation:** Memory and channel segregation between unique users.

---

### 💡 Future Refinement
Future iterations will focus on implementing a formal dynamic eviction policy for inactive sessions to further optimize long-term memory footprint.
