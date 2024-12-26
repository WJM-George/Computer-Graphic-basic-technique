# CSCI 420 Assignment 1
## Name: Wu Kam Man. USC ID: kammanwu@usc.edu
## Basic Description
This project is able to render different geometric shapes (points, lines, triangles) using vertex and fragment shaders.

## Features
- Render points, lines, and triangles.
- Smooth mode of triangle using vertex shaders.
- Can use Scale and Exponent to change the view of picture
- Camera control with transformations. Sample : rotation, translation, scaling.

## Controls:
- Press `ESC` to exit the program.
- Press `1` -> render points.
- Press `2` -> render lines.
- Press `3` -> render triangles.
- Press `4` -> smooth triangle. (With JetColor)
- Press `5` -> Render wireframe on the top of triangles.
- Use `+` and `-` to change scale.
- Press `9` and `0` to change exponent. (9 larger, 0 smaller)

### Point Mode
- In point mode, we iterate through all the points and store their positions using a unique pointer.
- Correpondingly we set the colors base on the point positions.

### Line Mode
- For line mode, we need to set up a VBO corresponding to the recorded positions of the points. And a corresponding VAO for binding.
- Lines are drawn by connecting the points we recorded in the previous step. 
- The unique pointer for line positions is a flat unique pointer. Then we record the drawing logic using a two-loop iteration that tracks the connections:
  - **Horizontal Lines**: In the iteration: connect `(i, j)` to `(i, j + 1)`
  - **Vertical Lines**: In the iteration: connect `(i, j)` to `(i + 1, j)`

#### Additional Implementation Notes
- For wireframe generation, we create two extra lines by determining the final positions for both horizontal and vertical lines.
- Finally, we iterate through each pixel in the final column and row to draw the two lines.
- For the exponent part, for my own convenience of coloring or strenching the structure, I turn the exponent factor into 1.1f.

### Triangle Mode
- Similar to line mode, we use a VBO for triangle rendering. And a corresponding VAO for binding.
- Triangles are formed by connecting three points at a time using the previously recorded positions.
- The drawing sequence is again tracked using a flat unique pointer, and we iterate through the positions to connect them appropriately.
  - **Triangle 1**: In the iteration: connect the points `(i, j)`, `(i + 1, j)`, and `(i, j + 1)`.
  - **Triangle 2**: In the iteration: connect the points `(i, j + 1)`, `(i + 1, j + 1)`, and `(i + 1, j)`.

### Smooth Mode:
- Instead of directly get the vbo of VertexDirection into the vertex shader. I prefer to update the position first inside cpu and then draw the triangle in gpu.
- Hence, similar to Triangle mode above, we use a single VBO for triangle rendering. And a corresponding VAO for binding.
- What is different is that we first update the point positions we used for getting the connection of triangles later on. 
i.e. The current point position is different from the one in point mode, The current one is a smooth version of the initial Position we
had in point mode.
- The drawing sequence is again tracked using a flat unique pointer, and we iterate through the positions to connect them appropriately.
  - Need to point out that the points here we connect is the smooth points we generated above.
  - **Triangle 1**: In the iteration: connect the points `(i, j)`, `(i + 1, j)`, and `(i, j + 1)`.
  - **Triangle 2**: In the iteration: connect the points `(i, j + 1)`, `(i + 1, j + 1)`, and `(i + 1, j)`.

### Extra Credit
- support: Render wireframe on top of solid triangles (use glPolygonOffset to avoid z-buffer fighting).
- support: Allow JetColorMap function to be used in the vertex shader.

## Notice
- The video and the pictures nearly have the same effect comparing the Sep 30th version and the Oct 7th version.
Hence I will keep the Sep 30th Version. (Make another video takes a long time. And I have a midterm tomorrow) .