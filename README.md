# STOMP Messaging Server

A high-performance, asynchronous, multi-threaded messaging server implementing the **STOMP protocol**. Designed for high throughput and reliability, the system pairs a Java-based Reactor/TPC backend with a high-concurrency native C++ client. Developed as part of the Systems Programming course at Ben-Gurion University (BGU).

## 🚀 Project Overview

This distributed system facilitates real-time communication between multiple clients and a central server. The architecture emphasizes thread safety, low-latency message routing, and robust socket lifecycle management.

## 👥 Development Team

* **Sabreen Abo Saheban:** Core Protocol Engine, C++ Client Event Loop, JSON Integration, Integration Testing, and Architectural Optimization.
* **Team Member:** Server Concurrency Layer, Thread-Safety Mechanics, and Locking Primitives.

---

## 🛠 Technical Contributions (Sabreen Abo Saheban)

### 1. Server-Side Protocol Engine

* **State Machine Architecture:** Engineered the protocol translation layer to deserialize raw network packets into validated STOMP frames (`CONNECT`, `SUBSCRIBE`, `SEND`, `DISCONNECT`).
* **Frame Serialization:** Developed a high-performance encoding layer ensuring strict adherence to protocol specifications.

### 2. Native C++ Concurrent Client

* **Asynchronous Event Loop:** Implemented a non-blocking client-side architecture, enabling simultaneous socket listening and UI/CLI interactions.
* **Dynamic Data Parsing:** Developed a robust JSON parsing sub-layer to translate complex news events into compliant STOMP text frames.

### 3. Integration & System Stability

* **End-to-End Verification:** Orchestrated the integration of the Java backend with the C++ client, conducting stress testing on shared state mechanics.
* **Concurrency Debugging:** Rigorously audited locking primitives and thread-safe collections to eliminate race conditions under high-concurrency loads.

---

## 🏗 Architectural Rationale

### Server-Side (Java)

* **`ConcurrentHashMap` Hierarchy:** Utilized for thread-safe session and topic management, ensuring $O(1)$ lookup times while maintaining high concurrency.
* **Inverted Topic-to-Session Mapping:** Optimized broadcast performance by shifting from $O(N * M)$ to $O(K)$ lookup complexity, directly targeting active subscribers.

### Client-Side (C++)

* **Buffered Stream Management:** Used dynamic memory buffers (`std::vector<char>`) for efficient streaming and packet boundary isolation.
* **Reliability:** Implemented a resilient reconnection algorithm to manage rapid session transitions and socket lifecycle states.

---

## ⚙️ Engineering Challenges & Resolution: The "Broken Pipe" Fix

During the integration phase, we identified a critical architectural bottleneck: **Intermittent `Broken pipe` errors** occurring during rapid `Login -> Logout -> Login` cycles.

* **Root Cause:** The issue stemmed from a race condition where the client attempted to initiate a new connection before the server had fully purged the socket teardown process and session registry associated with the previous `ID`.
* **The "Deep Dive" Refinement:** Even 4 months after successfully completing the course, I returned to this codebase to ensure it reached production-grade stability. I engineered and implemented a **Robust Reconnection Algorithm** on the client side. This logic introduces an incremental backoff mechanism (Exponential Delay), providing the server the necessary "breathing room" to finalize socket cleanup and memory eviction.
* **Result:** This post-course refinement achieved **100% stability** in session cycling, proving that true engineering excellence goes beyond meeting course requirements.

---

## 📝 Features & Protocol Compliance

* **Full Protocol Handshake:** Seamless execution of `CONNECT`, `SUBSCRIBE`, `SEND`, and `DISCONNECT` flows.
* **Data Injection:** Dynamic JSON parsing and event distribution.
* **Memory Isolation:** Strict channel and session segregation between unique users.

---

### 💡 Future Refinement

While the system handles multi-session lifecycles with high stability, future iterations will focus on implementing a formal dynamic eviction policy for inactive sessions to further optimize memory footprint.

---
