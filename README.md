# 🧠 Neuronix

**Neuronix** is software developed for educational purposes, focused on simulating computational neuroscience concepts.

---

## ⚠️ Important Warning Regarding Execution

This project **is not a program that you install on your system** (Linux, Windows or macOS).

The executable file ('neuronix') is saved **inside `bin/` folder** within the project's own directory. You will not find it in `/usr/bin/` or in "Program Files".

---

## 🚀 How to Compile and Run

To use the program, you must first compile it and then run it from the project's root folder.

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/Marshmallow-963/neurolab.git](https://github.com/Marshmallow-963/neurolab.git)
    cd neurolab
    ```

2.  **Compile the project:**
    The raylib and raygui files are already included in the project. You only need to have the `make` and `gcc` builds tools, and the base libraries that raylib depends on (such as X11, pthread, etc., which most Linux distributions already have or include in packages like `build-essential` or `libx11-dev`).

    ```bash
    make clean && make all
    ```

3.  **Run the program:**
    After compilation, the executable will be available in the `bin/` folder. To run it, use the following command from the root folder (`neurolab/`):
    ```bash
    ./bin/neurolab
    ```

---

## 🛠️ Implementation Notes

Neuronix is built in **C**, prioritizing performance and manual memory management to simulate complex neural networks efficiently. The architecture follows a modular design, separating the network topology (Graph) from the biological logic (Models).

### 1. Network Architecture (Graph Structure)
The simulation core is structured as a **Directed Graph**:
* **Vertices (Neurons):** Represented by the `NeuronNode` structure.
* **Edges (Synapses):** Represented by the `SynapseEdge` structure (adjacency list).

To support heterogeneous networks (networks containing different types of neurons simultaneously), we utilize **C-style Polymorphism**:
* The `NeuronNode` struct contains a generic `void *modelData` pointer.
* An `enum NeuronModel` flag identifies whether the data points to an `IzhikevichModel` or a `HodgkinHuxleyModel`.
* The `NetworkManager` handles the casting and dispatching of the appropriate update functions (`UpdateModel`) during the simulation loop.

### 2. Neuron Models
The software implements distinct mathematical models, each with its own internal state vectors and parameter buffers.

#### A. Izhikevich Model
Optimized for computational efficiency, allowing for large-scale simulations.
* **Mathematical Basis:** Solves a system of two Ordinary Differential Equations (ODEs) describing membrane potential ($v$) and a recovery variable ($u$).
* **Dynamics:** Implements a conditional reset mechanism:
    $$if \quad v \ge 30mV \quad then \quad \begin{cases} v \leftarrow c \\ u \leftarrow u + d \end{cases}$$
* **Implementation:** The state variables are updated using the **Runge-Kutta 4 (RK4)** numerical integration method for stability.

#### B. Hodgkin-Huxley Model
Focused on biophysical accuracy, modeling the ionic mechanisms of action potential initiation.
* **Mathematical Basis:** A 4-variable system ($V, m, h, n$) representing membrane voltage and the gating variables for Sodium ($Na^+$) and Potassium ($K^+$) channels.
* **Currents:** Explicitly calculates ionic currents:
    * $I_{Na}$ (Sodium Current)
    * $I_{K}$ (Potassium Current)
    * $I_{L}$ (Leak Current)
* **Implementation:** Due to the stiffness of these equations, the RK4 integrator is essential to maintain accuracy at reasonable time steps ($dt$).

### 3. Synaptic Modeling (Chemical Synapses)
Synapses are modeled not just as static weights, but as dynamic chemical connections involving neurotransmitter release and receptor kinetics.

* **Types:**
    * **AMPA:** Excitatory synapses (rapid kinetics).
    * **GABA-A:** Inhibitory synapses (slower kinetics).
* **Mechanism:**
    1.  **Presynaptic Trigger:** The model monitors the presynaptic voltage ($V_{pre}$). When it crosses a threshold, neurotransmitter concentration ($T$) in the cleft increases.
    2.  **Receptor Binding:** We simulate the fraction of open receptors ($r$) using first-order kinetics:
        $$\frac{dr}{dt} = \alpha T (1-r) - \beta r$$
    3.  **Postsynaptic Current:** The resulting current is injected into the target neuron:
        $$I_{syn} = g_{max} \cdot r \cdot (V_{post} - E_{rev})$$
* **Optimization (Pointer Wiring):** To ensure high performance, synapses hold **direct pointers** to the memory addresses of the pre-synaptic voltage and the post-synaptic current accumulator. This avoids expensive memory lookups during the critical path of the simulation loop.

### 4. Memory Management & Integration
* **RK4 Integrator:** A generic Runge-Kutta 4 implementation is used across all models to solve differential equations.
* **Manual Allocation:** All structures are dynamically allocated (`malloc`) in the heap to support runtime network resizing.
* **Lifecycle:** The `NetworkDestroy` function ensures a clean teardown, freeing memory in a specific order (Synapses $\to$ Models $\to$ Nodes $\to$ Network) to prevent memory leaks.

---

## 🎓 Authorship and Academic Context

This software was developed by an Undergraduate Research (IC) student from the **Computational Neuroscience Group** at the **Institute of Physics (IF)** of the **Federal University of Alagoas (Ufal)**.

The project was carried out under the supervision of Prof. Dr. Fernada Selingrad.
