# AMK-Renderer
Basic 3D software renderer written in pure C++

the goal of this project was to do everything from scratch
without using any external graphics libraries (eg. OpenGL, DirectX, ....).
by manuplating the pixels directly.

Features :
- basic 3D Rendering pipeline.
- UV, Texture and Normals mapping.
- specular highlighting.
- limited support for loading **.obj** file format :
  - the program uses custom commands written to **.obj files** to specify textures and other things.
  - the program can only process **Tri** and **Quad** faces, no support for **multi-vertex** faces.
- saving the render to an image.

you can [download](https://mega.nz/file/ARMUGYDZ#yLOK3QnlhP1PQRv6wEwiRY749WF6yeFRsDmEFo2RACE) it.

# Preview
![Capture](https://github.com/AliMustafa731/AMK-Renderer/assets/65477431/f96c1262-d5e8-4af3-bfe1-0e30f344fb01)

