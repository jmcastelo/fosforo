# What is videofeedback?

Imagine you had a videocamera and a television, connected in such a way that the television displays what the camera is capturing. If you pointed the camera directly to the television, what would the television display? Actually you would have built a videofeedback loop. Depending on parameters like the camera zoom, different kind of images would result. For instance, you could see a television containing a smaller television, containing in turn another television, and so on... 

A videofeedback loop is a system that processes images iteratively. Given an initial image displayed on the television, the camera captures it and modifies it adjusting things like its brightness, contrast or colors. Then the signal is transferred to the television, which can modify it in turn. Once the signal is converted into an image and the pixels on the screen are displaying it, a single iteration has happened and the process repeats.

# What is Fosforo?

It is an interactive tool to simulate videofeedback. It does not restrict itself to two image processing elements (camera and television), but tries to be more flexible by giving the user the possibility of selecting operations and connecting them as the user wants. These operations are represented by nodes in a graph. An image flows from node to node, being processed in each one before passing to the next one. The connections are represented by the edges of the graph and are unidirectional. Multiple inputs and outputs are possible.

## The flow: loops, edges and predges

Due to the discrete nature of computers, the image-processing flow of **fósforo** cannot be a continuous stream but must be performed in discrete steps known as iterations, which makes us think about how to treat loops.

A _loop_ is formed whenever two or more nodes in a graph are connected forming a closed path. If we select any node from a loop and follow its connections through the loop, the process will bring us back to the starting node. In other words, the input of any node depends on the output of the same node. This forces us to make the question: given a graph with loops, how do we understand the flow of the system?

We solve this issue by cutting the loops. Once cut, a loop is no longer a loop but a chain of operations that can be sequentially carried out during an iteration. For this purpose we define two kinds of connections: edges and _predges_. During an iteration, an edge links the output of a node to the input of another node, whereas a predge links a node's previous iteration output to the input of another node. A predge corresponds to a loop's cut connection and any loop must have at least one predge.

We can visualize the iterated system as a set of parallel planes. Imagine the graph representing the system, without its predges, as laying on a plane N which corresponds to iteration N. A similar parallel plane corresponding to iteration N+1 is located above the first one. The predges of nodes from the graph on plane N connect to those of plane N+1. Once iteration N is carried out, all nodes with predges on plane N send their output images to their connected nodes of plane N+1 and iteration N+1 starts.

## Seeds

Any iterated system needs an initial element to start the process, so the user can also add to the graph a kind of node that injects an image to the system. We name these nodes seeds. A seed can have multiple outputs but no inputs.

## Sorting algorithm

The order of the operations within an iteration is given by the following sorting algorithm. Any node with no inputs nor outputs is discarded. We select as initial nodes those which have no inputs but some outputs, and those whose inputs are all predges or edges connecting to seeds. These kinds of nodes do not depend on the output of any other node within an iteration, so they can be computed in the first place. The rest of the nodes can be computed only if their input nodes have been computed. So we check for this condition iteratively and when a node meets it, we set it as computed and add it to the sorted list. We do this while there are pending nodes.

## Screenshots

To do...

## Videos

To do...

# Compilation

## Build options

Useful environment variables to build and run.

```
QT_MEDIA_BACKEND=ffmpeg
QT_FFMPEG_ENCODING_HW_DEVICE_TYPES=vdpau
QT_FFMPEG_DECODING_HW_DEVICE_TYPES=vdpau
QT_FFMPEG_HW_ALLOW_PROFILE_MISMATCH=1
QT_FFMPEG_DEBUG=1
QT_QPA_PLATFORM=wayland
QT_ASSUME_STDERR_HAS_CONSOLE=1
```

# How to use Fosforo?

To do...

## User interface

To do...

# Operations

To do...

# Bibliography and links

"Gödel, Escher, Bach: an Eternal Golden Braid", Douglas R. Hofstadter.

# License

This software is open source and available under the GPLv3 License.
