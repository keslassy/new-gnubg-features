
#include "config.h"
#include "legacyGLinc.h"
#include <stdlib.h>
#include "fun3d.h"

extern void InitMatStacks();

void OglModelInit(OglModel* oglModel)
{
	oglModel->data = NULL;
	oglModel->dataLength = 0;
	oglModel->modelUsesTexture = 0;

	InitMatStacks();
}

void OglModelAlloc(OglModel* oglModel)
{
	oglModel->data = malloc(sizeof(float) * oglModel->dataLength);
	oglModel->dataLength = 0;
}

void OglModelDraw(const OglModel* oglModel)
{
	float* data = oglModel->data;

	int vertexSize = 6;
	if (oglModel->modelUsesTexture)
		vertexSize += 2;

	glBegin(GL_TRIANGLES);
	for (int vertex = 0; vertex < oglModel->dataLength / vertexSize; vertex++)
	{
		if (oglModel->modelUsesTexture)
		{
			glTexCoord2f(data[0], data[1]);
			data += 2;
		}
		glNormal3f(data[0], data[1], data[2]);
		glVertex3f(data[3], data[4], data[5]);
		data += 6;
	}
	glEnd();
}
