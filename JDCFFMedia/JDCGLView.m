//
//  JDCGLView.m
//  JDCFFPlayer
//
//  Created by maochengrui on 05/05/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#import "JDCGLView.h"
#import <OpenGLES/ES2/gl.h>
#import "JDCAVFrame.h"

//Select one of the Texture mode (Set '1'):
#define TEXTURE_DEFAULT   1
//Rotate the texture
#define TEXTURE_ROTATE    0
//Show half of the Texture
#define TEXTURE_HALF      0

#define ATTRIB_VERTEX 0
#define ATTRIB_TEXTURE 1

@interface JDCGLView(){
    GLuint _program;
    GLuint _id_y, _id_u, _id_v; // Texture id
    GLuint _textureUniformY, _textureUniformU,_textureUniformV;
    uint8_t *plane[3];
    
    GLuint          _framebuffer;
    GLuint          _renderbuffer;
    GLint           _backingWidth;
    GLint           _backingHeight;
    GLint           _uniformMatrix;
    GLfloat         _vertices[8];
}

@end

@implementation JDCGLView

- (instancetype)initWithFrame:(CGRect)frame
{
    if (self = [super initWithFrame:frame]) {
        [self commomInit];
    }
    
    return self;
}

- (instancetype)init
{
    if (self = [super init]) {
        [self commomInit];
    }
    
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder
{
    if (self = [super initWithCoder:aDecoder]) {
        [self commomInit];
    }
    
    return self;
}

- (void)commomInit
{
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    self.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.drawableMultisample = GLKViewDrawableMultisample4X;
    
    [self initShaders];
    
    [self setNeedsDisplay];
}


- (void)render:(JDCAVFrame *)frame
{
    GLsizei pixel_h = frame.avFrame->linesize[0];
    GLsizei pixel_w = frame.avFrame->linesize[0];
    
    for(int i = 0 ; i < 3 ;i++){
        plane[i] = frame.avFrame->data[i];   
    }
    
    //Clear
    glClearColor(0.0,255,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //Y
    //
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id_y);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w,
                 pixel_h,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[0]);
    glUniform1i(_textureUniformY, 0);
    //U
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _id_u);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w/2,
                 pixel_h/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[1]);
    
    glUniform1i(_textureUniformU, 1);
    //V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _id_v);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w/2,
                 pixel_h/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[2]);
    
    glUniform1i(_textureUniformV, 2);
    
    // Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    
    //Clear
    glClearColor(0.0,255,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //Y
    //
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id_y);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w,
                 pixel_h,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[0]);
    glUniform1i(_textureUniformY, 0);
    //U
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _id_u);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w/2,
                 pixel_h/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[1]);
    
    glUniform1i(_textureUniformU, 1);
    //V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _id_v);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w/2,
                 pixel_h/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[2]);
    
    glUniform1i(_textureUniformV, 2);
    
    // Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
}

- (void)initShaders
{
    GLint vShaderCompiled , fShaderCompiled , linkedShader;
    GLint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    NSString *vPath = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
    NSString *fPath = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
    NSString *vSource = [NSString stringWithContentsOfFile:vPath usedEncoding:nil error:nil];
    NSString *fSource = [NSString stringWithContentsOfFile:fPath usedEncoding:nil error:nil];
    
    const char *vShaderUTF8Str = [vSource UTF8String];
    const char *fShaderUTF8Str = [fSource UTF8String];
    
    glShaderSource(vShader, 1, &vShaderUTF8Str, NULL);
    glShaderSource(fShader, 1, &fShaderUTF8Str, NULL);
    
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &vShaderCompiled);
    
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &fShaderCompiled);
    
    GLint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    
    glBindAttribLocation(program, ATTRIB_VERTEX , "vertexIn");
    glBindAttribLocation(program, ATTRIB_VERTEX , "textureIn");
    
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkedShader);
    glUseProgram(program);
    _program = program;
    
    //Get Uniform Variables Location
    _textureUniformY = glGetUniformLocation(program, "tex_y");
    _textureUniformU = glGetUniformLocation(program, "tex_u");
    _textureUniformV = glGetUniformLocation(program, "tex_v");
    
    
#if TEXTURE_ROTATE
    static const GLfloat vertexVertices[] = {
        -1.0f, -0.5f,
        0.5f, -1.0f,
        -0.5f,  1.0f,
        1.0f,  0.5f,
    };
#else
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };
#endif
    
#if TEXTURE_HALF
    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        0.5f,  1.0f,
        0.0f,  0.0f,
        0.5f,  0.0f,
    };
#else
    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };
#endif

    
    //Set Arrays
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    //Enable it
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    
    
    //Init Texture
    glGenTextures(1, &_id_y);
    glBindTexture(GL_TEXTURE_2D, _id_y);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &_id_u);
    glBindTexture(GL_TEXTURE_2D, _id_u);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &_id_v);
    glBindTexture(GL_TEXTURE_2D, _id_v);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
}


- (void)drawRect:(CGRect)rect
{
    [super drawRect:rect];
    
    GLsizei pixel_h = 100;
    GLsizei pixel_w = 100;
    
    //Clear
    glClearColor(0.0,255,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //Y
    //
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id_y);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w,
                 pixel_h,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[0]);
    glUniform1i(_textureUniformY, 0);
    //U
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _id_u);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w/2,
                 pixel_h/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[1]);
    
    glUniform1i(_textureUniformU, 1);
    //V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _id_v);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 pixel_w/2,
                 pixel_h/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 plane[2]);
    
    glUniform1i(_textureUniformV, 2);
    
    // Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

@end
