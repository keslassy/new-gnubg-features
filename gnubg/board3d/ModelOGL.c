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

void OglModelInit(ModelManager* modelHolder, int modelNumber)
{
	modelHolder->models[modelNumber].data = NULL;
	modelHolder->models[modelNumber].dataLength = 0;
	modelHolder->models[modelNumber].modelUsesTexture = 0;

	InitMatStacks();

	modelHolder->numModels++;
}

void OglModelAlloc(ModelManager* modelHolder, int modelNumber)
{
	modelHolder->totalNumVertices += modelHolder->models[modelNumber].dataLength;

	modelHolder->models[modelNumber].data = malloc(sizeof(float) * modelHolder->models[modelNumber].dataLength);
	modelHolder->models[modelNumber].dataLength = 0;
}

void OglModelDraw(const ModelManager* modelManager, int modelNumber)
{
	float* data = &modelManager->vertexData[modelManager->models[modelNumber].dataStart];
	int dataLength = modelManager->models[modelNumber].dataLength;

	int vertexSize = 6;
	if (modelManager->models[modelNumber].modelUsesTexture)
		vertexSize += 2;

	glBegin(GL_TRIANGLES);
	for (int vertex = 0; vertex < dataLength / vertexSize; vertex++)
	{
		if (modelManager->models[modelNumber].modelUsesTexture)
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

void ModelManagerInit(ModelManager* modelHolder)
{
	modelHolder->allocNumVertices = 0;
	modelHolder->vertexData = NULL;
}

void ModelManagerStart(ModelManager* modelHolder)
{
	modelHolder->totalNumVertices = 0;
	modelHolder->numModels = 0;
}

void ModelManagerCopyModelToBuffer(ModelManager* modelHolder, int modelNumber)
{
	memcpy(&modelHolder->vertexData[modelHolder->models[modelNumber].dataStart], modelHolder->models[modelNumber].data, modelHolder->models[modelNumber].dataLength * sizeof(float));
	free(modelHolder->models[modelNumber].data);
	modelHolder->models[modelNumber].data = NULL;
}

void ModelManagerCreate(ModelManager* modelHolder)
{
	int model;
	if (modelHolder->totalNumVertices > modelHolder->allocNumVertices)
	{
		modelHolder->allocNumVertices = modelHolder->totalNumVertices;
		free(modelHolder->vertexData);
		modelHolder->vertexData = malloc(sizeof(float) * modelHolder->allocNumVertices);
		g_assert(modelHolder->vertexData != NULL);
	}
	/* Copy data into one big buffer */
	int vertexPos = 0;
	for (model = 0; model < modelHolder->numModels; model++)
	{
		modelHolder->models[model].dataStart = vertexPos;
		ModelManagerCopyModelToBuffer(modelHolder, model);
		vertexPos += modelHolder->models[model].dataLength;
	}
	g_assert(vertexPos == modelHolder->totalNumVertices);
}
