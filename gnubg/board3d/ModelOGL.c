/*
 * Copyright (C) 2019 Jon Kinsey <jonkinsey@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "config.h"
#include "legacyGLinc.h"
#include <stdlib.h>
#include "fun3d.h"

extern void InitMatStacks(void);

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
