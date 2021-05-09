## Compiling
I use Windows and VS. All dependencies are submodules in `src/lib`.   


## Progress Description & Overview
#### Describe the overall goal of your project: What were you aiming to accomplish and how much of this goal you actually achieved?
The main goal in a word I wanted to achieve was shadowmaps. Directional shadowmaps are the direct version of this, but I additionally wanted to add point light shadowmaps. After this, I wanted to implement more flexible level creation and mesh generation, which would lead into being able to update the map with the program open (without having to reload anything). The goal here was to do this without regenerating the entire level mesh, mostly for "I did this" reasons than practical ones. 

I achieved directional shadowmaps which were easily updatable in real time, and would track all objects. I achieved this by rendering a depth map from the directional light's perspective, and of course testing against this shadowmap in the fragment shader. This product was a success, it did exactly what I wanted to in the way that I expected to without compromises.
#### What were they main obstacles you encountered along the way?
## Connection to Our Class
#### In what ways did your project demonstrate various aspects of computer graphics we have discussed in this class?
## Key Features & Images
#### Describe the various key features of your final working system. Show several images of your project working. Highlight the various features with different images. 
## Future Work
#### In what ways could your project be extended going forward? 
