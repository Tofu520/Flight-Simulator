#include "VAO.h"

VAO::VAO(){
    glGenVertexArrays(1,&ID);
}

/*
When reading attribute layout, 
read numComponents values of type type, 
starting at offset, stepping stride bytes per vertex.
*/
void VAO::LinkAttrib(VBO& VBO,GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void * offset){
    VBO.Bind();

    //tells how to interpret the bytes all the bytes sent for the vertex
    //pos of vertex, number of values, type of values, only for ints, amount of data between each vertex,offset
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset); //works with VAO together
    
    //read attribute data from attribute slot 0 of the currently bound VAO and pass it into the vertex shader.
    glEnableVertexAttribArray(layout); 
    VBO.Unbind();
}

void VAO::Bind(){
    glBindVertexArray(ID);
}

void VAO::Unbind(){
    glBindVertexArray(0);
}

void VAO::Delete(){
    glDeleteVertexArrays(1,&ID);
}

