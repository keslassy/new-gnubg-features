
typedef float GLfloat;

#define GL_MODELVIEW                      0x1700
#define GL_PROJECTION                     0x1701
#define GL_TEXTURE                        0x1702

#define GL_QUADS                          0x0007
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUAD_STRIP                     0x0008

#define GL_LEQUAL                         0x0203
#define GL_ALWAYS                         0x0207
#define GL_TRUE                           1
#define GL_FALSE                          0

void SHIMsetMaterial(const Material* pMat);
void SHIMglBegin(GLenum mode);
void SHIMglEnd();
void SHIMglMatrixMode(GLenum mode);
void SHIMglPushMatrix();
void SHIMglPopMatrix();
void SHIMglRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void SHIMglTranslatef(GLfloat x, GLfloat y, GLfloat z);
void SHIMglScalef(GLfloat x, GLfloat y, GLfloat z);
void SHIMglNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
void SHIMglTexCoord2f(GLfloat s, GLfloat t);
void SHIMglVertex2f(GLfloat x, GLfloat y);
void SHIMglVertex3f(GLfloat x, GLfloat y, GLfloat z);

#define setMaterial SHIMsetMaterial
#define glPushMatrix SHIMglPushMatrix
#define glPopMatrix SHIMglPopMatrix
#define glTranslatef SHIMglTranslatef
#define glScalef SHIMglScalef
#define glNormal3f SHIMglNormal3f
#define glTexCoord2f SHIMglTexCoord2f
#define glVertex2f SHIMglVertex2f
#define glVertex3f SHIMglVertex3f
#define glBegin SHIMglBegin
#define glEnd SHIMglEnd
#define glMatrixMode SHIMglMatrixMode
#define glRotatef SHIMglRotatef
