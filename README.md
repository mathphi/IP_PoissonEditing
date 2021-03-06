# Image Processing Project - Poisson Image Editing
Image composition (collage) with Poisson blending.

***NOTE: This repository uses the [Eigen](https://gitlab.com/libeigen/eigen "Eigen") project as a submodule. To clone the project with its submodules, use:***

`git clone --recurse-submodules https://github.com/mathphi/IP_PoissonEditing.git`

![Poisson Image Blending - Capture](PoissonImageEditing/PoissonImageBlending-Capture.jpg "Poisson Image Blending - Capture")

#### Description
- Manually compose pieces from several images
- Automatically blend these pieces and produce an output image

#### Technologies
- Define regions of interest in each input image that need to appear in the final result
- Poisson blending, Edge detection/segmentation starting from several color images

#### Bonus targets
- Implement an intuitive GUI that lets users combine images in interesting ways.
- ~~Explore video editing (only a few frames)~~

#### References
- P. Prez, M. Gangnet, and A. Blake, Poisson image editing, ACM Transactions on Graphics
- "Discrete Poisson equation", https://en.wikipedia.org/wiki/Discrete_Poisson_equation
- PPPW, "Poisson Image Editing", https://github.com/PPPW/poisson-image-editing
- R.-C. Cocioban, "Poisson Image Editing", https://github.com/ralucacocioban/PoissonImageEditing
- W. Emmanuel, "Poisson Image Editing", https://github.com/willemmanuel/poisson-image-editing
