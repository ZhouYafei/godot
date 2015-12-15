// //
// // $id: SkeletonBinary.c 2014-08-06 zhongfengqu $
// //

#include "spine/SkeletonBinary.h"
#include "spine/extension.h"

// //#include <stdlib.h>
// //#include <stdio.h>
// //#include <math.h>

// #define TIMELINE_SCALE		0
// #define TIMELINE_ROTATE		1
// #define TIMELINE_TRANSLATE	2
// #define TIMELINE_ATTACHMENT	3
// #define TIMELINE_COLOR		4
// #define TIMELINE_FLIPX		5
// #define TIMELINE_FLIPY		6

// typedef enum {
// 	SP_CURVE_LINEAR,
// 	SP_CURVE_STEPPED,
// 	SP_CURVE_BEZIER,
// } spCurveType;

// typedef struct {
// 	spSkeletonJson *json;
// 	float scale;
// 	char *rawdata;
// 	char *reader;
// //	char **strs;
// } spSkeletonBinary;

// #define true 1
// #define false 0

// #define MIN(a, b) (((a)<(b))?(a):(b))
// #define MAX(a, b) (((a)>(b))?(a):(b))
// #define READ() (((int)*self->reader++) & 0xFF)

// static int readBoolean(spSkeletonBinary *self) {

// 	int ch = READ();
// 	return ch != 0;
// }

// static char readChar(spSkeletonBinary *self) {

// 	int ch = READ();
// 	return (char)(ch);
// }

// static short readShort(spSkeletonBinary *self) {

// 	int ch1 = READ();
// 	int ch2 = READ();
// 	return (short)((ch1 << 8) + (ch2 << 0));
// }

// static int readInt(spSkeletonBinary *self, int optimized) {

// 	int b = readChar(self);
// 	int result = b & 0x7F;
// 	if ((b & 0x80) != 0) {
// 		b = readChar(self);
// 		result |= (b & 0x7F) << 7;
// 		if ((b & 0x80) != 0) {
// 			b = readChar(self);
// 			result |= (b & 0x7F) << 14;
// 			if ((b & 0x80) != 0) {
// 				b = readChar(self);
// 				result |= (b & 0x7F) << 21;
// 				if ((b & 0x80) != 0) {
// 					b = readChar(self);
// 					result |= (b & 0x7F) << 28;
// 				}
// 			}
// 		}
// 	}
// 	return optimized ? result : ((result >> 1) ^ -(result & 1));
// }

// static int readInt32(spSkeletonBinary *self) {

// 	int ch1 = READ();
// 	int ch2 = READ();
// 	int ch3 = READ();
// 	int ch4 = READ();
// 	return ((ch1 << 24) | (ch2 << 16) | (ch3 << 8) | (ch4 << 0));
// }

// static float readFloat(spSkeletonBinary *self) {

// 	union {
// 		float f;
// 		int i;
// 	} u;

// 	u.i = readInt32(self);
// 	return u.f;
// }

// static const char *readString(spSkeletonBinary *self) {

// 	int charCount = readInt(self, true);
// 	switch (charCount) {

// 	case 0:
// 		return NULL;
// 	case 1:
// 		return "";
// 	}
// 	charCount--;

// 	static char *chars = NULL;
// 	static int chars_length = 0;
// 	if (chars_length < charCount) {

// 		chars = malloc(charCount + 1);
// 		chars_length = charCount;
// 	}
// 	// Try to read 7 bit ASCII chars.
// 	int charIndex = 0;
// 	int b = 0;
// 	while (charIndex < charCount) {

// 		b = readChar(self);
// 		if (b > 127) break;
// 		chars[charIndex++] = (char)b;
// 	}
// 	chars[charIndex] = '\0';
// 	// If a char was not ASCII, finish with slow path.
// 	//if (charIndex < charCount) readUtf8_slow(charCount, charIndex, b);
// 	return chars;
// }

// static float *readFloats(spSkeletonBinary *self, float scale, size_t *length) {

// 	float *arr;
// 	int i;
// 	int n = readInt(self, true);
// 	*length = n;
// 	arr = MALLOC(float, n);
// 	for (i = 0; i < n; i++)
// 		arr[i] = readFloat(self) * scale;

// 	return arr;
// }

// static int *readShorts(spSkeletonBinary *self, size_t *length) {

// 	int *arr;
// 	int i;
// 	int n = readInt(self, true);
// 	*length = n;
// 	arr = MALLOC(int, n);
// 	for (i = 0; i < n; i++)
// 		arr[i] = readShort(self);

// 	return arr;
// }

// static void readColor(spSkeletonBinary *self, float *r, float *g, float *b, float *a) {

// 	*r = READ() / (float)255;
// 	*g = READ() / (float)255;
// 	*b = READ() / (float)255;
// 	*a = READ() / (float)255;
// }

// static spAttachment *readAttachment(spSkeletonBinary *self, spSkin *Skin, const char *attachmentName, int nonessential) {

// 	float scale = self->scale;
// 	const char *name = readString(self);
// 	if (name == NULL) name = attachmentName;

// 	switch (readChar(self)) {

// 		case SP_ATTACHMENT_REGION: {

// 			spAttachment *attachment;
// 			spRegionAttachment *region;
// 			const char *path = readString(self);
// 			if (path == NULL) path = name;

// 			attachment = spAttachmentLoader_newAttachment(self->json->attachmentLoader, 
// 				Skin, SP_ATTACHMENT_REGION, attachmentName, path);
// 			region = SUB_CAST(spRegionAttachment, attachment);
// 			if (path) MALLOC_STR(region->path, path);

// 			region->x = readFloat(self) * scale;
// 			region->y = readFloat(self) * scale;
// 			region->scaleX = readFloat(self);
// 			region->scaleY = readFloat(self);
// 			region->rotation = readFloat(self);
// 			region->width = readFloat(self) * scale;
// 			region->height = readFloat(self) * scale;
// 			readColor(self, &region->r, &region->g, &region->b, &region->a);

// 			spRegionAttachment_updateOffset(region);

// 			return SUPER_CAST(spAttachment, region);
// 		}
// 		case SP_ATTACHMENT_BOUNDING_BOX:  {

// 			size_t length;

// 			spAttachment *attachment = spAttachmentLoader_newAttachment(self->json->attachmentLoader,
// 				Skin, SP_ATTACHMENT_BOUNDING_BOX, attachmentName, NULL);
// 			spBoundingBoxAttachment *box = SUB_CAST(spBoundingBoxAttachment, attachment);

// 			box->vertices = readFloats(self, scale, &length);
// 			box->verticesCount = (int)length;

// 			return SUPER_CAST(spAttachment, box);
// 		}
// 		case SP_ATTACHMENT_MESH:  {

// 			size_t length;
// 			spAttachment *attachment;
// 			spMeshAttachment *mesh;
// 			const char *path = readString(self);
// 			if (path == NULL) path = name;

// 			attachment = spAttachmentLoader_newAttachment(self->json->attachmentLoader, 
// 				Skin, SP_ATTACHMENT_MESH, attachmentName, path);
// 			mesh = SUB_CAST(spMeshAttachment, attachment);
// 			if (path) MALLOC_STR(mesh->path, path);

// 			mesh->regionUVs = readFloats(self, 1, &length);
// 			mesh->triangles = readShorts(self, &length);
// 			mesh->trianglesCount = (int)length;
// 			mesh->vertices = readFloats(self, scale, &length);
// 			mesh->verticesCount = (int)length;
// 			readColor(self, &mesh->r, &mesh->g, &mesh->b, &mesh->a);
// 			mesh->hullLength = readInt(self, true) * 2;

// 			spMeshAttachment_updateUVs(mesh);

// 			return SUPER_CAST(spAttachment, mesh);
// 		}
// 		case SP_ATTACHMENT_SKINNED_MESH: {

// 			int verticesCount, b, w, nn, i;
// 			size_t length;
// 			spAttachment *attachment;
// 			spSkinnedMeshAttachment *mesh;
// 			const char *path = readString(self);
// 			if (path == NULL) path = name;

// 			attachment = spAttachmentLoader_newAttachment(self->json->attachmentLoader, 
// 				Skin, SP_ATTACHMENT_SKINNED_MESH, attachmentName, path);
// 			mesh = SUB_CAST(spSkinnedMeshAttachment, attachment);
// 			if (path) MALLOC_STR(mesh->path, path);
			
// 			mesh->regionUVs = readFloats(self, 1, &length);
// 			mesh->uvsCount = (int)length;
// 			mesh->triangles = readShorts(self, &length);
// 			mesh->trianglesCount = (int)length;

// 			verticesCount = readInt(self, true);

// 			mesh->weightsCount = mesh->uvsCount * 3 * 3;
// 			mesh->weights = CALLOC(float, mesh->weightsCount);
// 			mesh->bonesCount = mesh->uvsCount * 3;
// 			mesh->bones = CALLOC(int, mesh->bonesCount);
// 			for (i = 0, b = 0, w = 0; i < verticesCount; i++) {

// 				int boneCount = (int)readFloat(self);
// 				mesh->bones[b++] = boneCount;
// 				for (nn = i + boneCount * 4; i < nn; i += 4, ++b, w += 3) {

// 					mesh->bones[b] = (int)readFloat(self);
// 					mesh->weights[w] = readFloat(self) * scale;
// 					mesh->weights[w + 1] = readFloat(self) * scale;
// 					mesh->weights[w + 2] = readFloat(self);
// 				}
// 			}
// 			readColor(self, &mesh->r, &mesh->g, &mesh->b, &mesh->a);
// 			mesh->hullLength = readInt(self, true) * 2;

// 			spSkinnedMeshAttachment_updateUVs(mesh);

// 			return SUPER_CAST(spAttachment, mesh);
// 		}
// 	}

// 	return NULL;
// }

// static spSkin *readSkin(spSkeletonBinary *self, const char *skinName, int nonessential) {

// 	spSkin *skin;
// 	int i;

// 	int slotCount = readInt(self, true);
// 	if (slotCount == 0)
// 		return NULL;

// 	skin = spSkin_create(skinName);
// 	for (i = 0; i < slotCount; i++) {

// 		int ii, nn;
// 		int slotIndex = readInt(self, true);
// 		for (ii = 0, nn = readInt(self, true); ii < nn; ii++) {

// 			const char *name = readString(self);
// 			spSkin_addAttachment(skin, slotIndex, name, readAttachment(self, skin, name, nonessential));
// 		}
// 	}

// 	return skin;
// }

// static void readCurve(spSkeletonBinary *self, spCurveTimeline *timeline, int frameIndex) {

// 	spCurveType type = (spCurveType)readChar(self);
// 	if (type == SP_CURVE_STEPPED) {

// 		spCurveTimeline_setStepped(timeline, frameIndex);
// 	} else if (type == SP_CURVE_BEZIER) {

// 		float v1 = readFloat(self);
// 		float v2 = readFloat(self);
// 		float v3 = readFloat(self);
// 		float v4 = readFloat(self);
// 		spCurveTimeline_setCurve(timeline, frameIndex, v1, v2, v3, v4);
// 	}
// }

// static void readAnimation(spSkeletonBinary *self, spSkeletonData *skeletonData, const char *name) {

// 	int i, ii, n, nn;
// 	float scale = self->scale;
// 	spAnimation *animation = spAnimation_create(name, readInt(self, true));
// 	animation->timelinesCount = 0;

// 	// Slot timelines
// 	n = readInt(self, true);
// 	for (i = 0; i < n; i++) {

// 		int slotIndex = readInt(self, true);
// 		nn = readInt(self, true);
// 		for (ii = 0; ii < nn; ii++) {

// 			int frameIndex;
// 			int timelineType = readChar(self);
// 			int framesCount = readInt(self, true);

// 			switch (timelineType) {

// 				case TIMELINE_COLOR: {

// 					spColorTimeline *timeline = spColorTimeline_create(framesCount);
// 					timeline->slotIndex = slotIndex;
// 					for (frameIndex = 0; frameIndex < framesCount; frameIndex++) {

// 						float time = readFloat(self);
// 						float r, g, b, a;
// 						readColor(self, &r, &g, &b, &a);
// 						spColorTimeline_setFrame(timeline, frameIndex, time, r, g, b, a);
// 						if (frameIndex < framesCount - 1)
// 							readCurve(self, SUPER(timeline), frameIndex);
// 					}
// 					animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 					animation->duration = MAX(animation->duration, timeline->frames[framesCount * 5 - 5]);
// 					break;
// 				}
// 				case TIMELINE_ATTACHMENT:
// 				{
// 					spAttachmentTimeline *timeline = spAttachmentTimeline_create(framesCount);
// 					timeline->slotIndex = slotIndex;
// 					for (frameIndex = 0; frameIndex < framesCount; frameIndex++) {

// 						float time = readFloat(self);
// 						spAttachmentTimeline_setFrame(timeline, frameIndex, time, readString(self));
// 					}
// 					animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 					animation->duration = MAX(animation->duration, timeline->frames[framesCount - 1]);
// 					break;
// 				}
// 			}
// 		}
// 	}

// 	// Bone timelines.
// 	n = readInt(self, true);
// 	for (i = 0; i < n; i++) {

// 		int boneIndex = readInt(self, true);
// 		nn = readInt(self, true);
// 		for (ii = 0; ii < nn; ii++) {

// 			int frameIndex;
// 			spTimelineType timelineType = (spTimelineType)readChar(self);
// 			int framesCount = readInt(self, true);
// 			switch (timelineType) {

// 				case TIMELINE_ROTATE: {

// 					spRotateTimeline *timeline = spRotateTimeline_create(framesCount);
// 					timeline->boneIndex = boneIndex;
// 					for (frameIndex = 0; frameIndex < framesCount; frameIndex++) {

// 						float time = readFloat(self);
// 						float angle = readFloat(self);
// 						spRotateTimeline_setFrame(timeline, frameIndex, time, angle);
// 						if (frameIndex < framesCount - 1)
// 							readCurve(self, SUPER(timeline), frameIndex);
// 					}
// 					animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 					animation->duration = MAX(animation->duration, timeline->frames[framesCount * 2 - 2]);
// 					break;
// 				}
// 				case TIMELINE_TRANSLATE:
// 				case TIMELINE_SCALE: {

// 					spTranslateTimeline *timeline;
// 					float timelineScale = 1;
// 					if (timelineType == TIMELINE_SCALE) {

// 						timeline = spScaleTimeline_create(framesCount);
// 					} else {

// 						timeline = spTranslateTimeline_create(framesCount);
// 						timelineScale = scale;
// 					}
// 					timeline->boneIndex = boneIndex;
// 					for (frameIndex = 0; frameIndex < framesCount; frameIndex++) {

// 						float time = readFloat(self);
// 						float x = readFloat(self) * timelineScale;
// 						float y = readFloat(self) * timelineScale;
// 						spTranslateTimeline_setFrame(timeline, frameIndex, time, x, y);
// 						if (frameIndex < framesCount - 1)
// 							readCurve(self, SUPER(timeline), frameIndex);
// 					}
// 					animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 					animation->duration = MAX(animation->duration, timeline->frames[framesCount * 3 - 3]);
// 					break;
// 				}
// 				case TIMELINE_FLIPX:
// 				case TIMELINE_FLIPY: {

// 					spFlipTimeline *timeline = spFlipTimeline_create(framesCount, timelineType == TIMELINE_FLIPX);
// 					timeline->boneIndex = boneIndex;
// 					for (frameIndex = 0; frameIndex < framesCount; frameIndex++) {

// 						float time = readFloat(self);
// 						spFlipTimeline_setFrame(timeline, frameIndex, time, readBoolean(self));
// 					}
// 					animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 					animation->duration = MAX(animation->duration, timeline->frames[framesCount * 2 - 2]);
// 					break;
// 				}
// 				default:
// 					break;
// 			}
// 		}
// 	}
	
// 	//ik timelines
// 	n = readInt(self, true);
// 	for (i = 0; i < n; i++) {

// 		int frameIndex;
// 		int index = readInt(self, true);
// 		int framesCount = readInt(self, true);
// 		spIkConstraintTimeline *timeline = spIkConstraintTimeline_create(framesCount);
// 		timeline->ikConstraintIndex = index;
// 		for (frameIndex = 0; frameIndex < framesCount; frameIndex++) {

// 			float time = readFloat(self);
// 			float mix = readFloat(self);
// 			int bendPositive = readBoolean(self);
// 			spIkConstraintTimeline_setFrame(timeline, frameIndex, time, mix, bendPositive);
// 			if (frameIndex < framesCount - 1)
// 				readCurve(self, SUPER(timeline), frameIndex);
// 		}
// 		animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 		animation->duration = MAX(animation->duration, timeline->frames[framesCount * 3 - 3]);
// 	}

// 	// FFD timelines
// 	n = readInt(self, true);
// 	for (i = 0; i < n; i++) {

// 		spSkin *skin = skeletonData->skins[readInt(self, true)];
// 		nn = readInt(self, true);
// 		for (ii = 0; ii < nn; ii++) {

// 			int slotIndex = readInt(self, true);
// 			int iii, nnn;
// 			nnn = readInt(self, true);
// 			for (iii = 0; iii < nnn; iii++) {

// 				int frameIndex = 0;
// 				int framesCount = 0;
// 				int verticesCount = 0;		 
// 				float *tempVertices;
// 				spFFDTimeline *timeline;

// 				spAttachment *attachment = spSkin_getAttachment(skin, slotIndex, readString(self));
// 				if (attachment->type == SP_ATTACHMENT_MESH)
// 					verticesCount = SUB_CAST(spMeshAttachment, attachment)->verticesCount;
// 				else if (attachment->type == SP_ATTACHMENT_SKINNED_MESH)
// 					verticesCount = SUB_CAST(spSkinnedMeshAttachment, attachment)->weightsCount / 3 * 2;

// 				framesCount = readInt(self, true);
// 				timeline = spFFDTimeline_create(framesCount, verticesCount);
// 				timeline->slotIndex = slotIndex;
// 				timeline->attachment = attachment;

// 				tempVertices = MALLOC(float, verticesCount);
// 				for (frameIndex = 0; frameIndex < framesCount; frameIndex++) {

// 					float *frameVertices;
// 					float time = readFloat(self);
// 					int start = readInt(self, true);
// 					int end = readInt(self, true);

// 					if (end == 0) {

// 						if (attachment->type == SP_ATTACHMENT_MESH)
// 							frameVertices = SUB_CAST(spMeshAttachment, attachment)->vertices;
// 						else {
// 							frameVertices = tempVertices;
// 							memset(frameVertices, 0, sizeof(float) * verticesCount);
// 						}
// 					} else {

// 						int v;
// 						frameVertices = tempVertices;
// 						end += start;
// 						if (scale == 1) {

// 							for (v = start; v < end; v++)
// 								frameVertices[v] = readFloat(self);
// 						} else {

// 							for (v = start; v < end; v++)
// 								frameVertices[v] = readFloat(self) * scale;
// 						}
// 						memset(frameVertices + v, 0, sizeof(float) * (verticesCount - v));
// 						if (attachment->type == SP_ATTACHMENT_MESH)  {

// 							float *meshVertices = SUB_CAST(spMeshAttachment, attachment)->vertices;
// 							for (v = 0; v < verticesCount; ++v)
// 								frameVertices[v] += meshVertices[v];
// 						}
// 					}

// 					spFFDTimeline_setFrame(timeline, frameIndex, time, frameVertices);
// 					if (frameIndex < framesCount - 1)
// 						readCurve(self, SUPER(timeline), frameIndex);
// 				}
// 				FREE(tempVertices);
// 				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 				animation->duration = MAX(animation->duration, timeline->frames[framesCount - 1]);
// 			}
// 		}
// 	}

// 	// Draw order timeline
// 	n = readInt(self, true);
// 	if (n > 0) {

// 		int* drawOrder = 0;
// 		spDrawOrderTimeline *timeline = spDrawOrderTimeline_create(n, skeletonData->slotsCount);
// 		int slotCount = skeletonData->slotsCount;
// 		int frameIndex;
// 		for (frameIndex = 0; frameIndex < n; frameIndex++) {

// 			int originalIndex = 0, unchangedIndex = 0;
// 			int offsetCount = readInt(self, true);
// 			int *unchanged = MALLOC(int, skeletonData->slotsCount - offsetCount);
// 			drawOrder = MALLOC(int, skeletonData->slotsCount);
// 			for (ii = slotCount - 1; ii >= 0; ii--)
// 				drawOrder[ii] = -1;
// 			for (ii = 0; ii < offsetCount; ii++) {

// 				int slotIndex = readInt(self, true);
// 				while (originalIndex != slotIndex)
// 					unchanged[unchangedIndex++] = originalIndex++;
// 				drawOrder[originalIndex + readInt(self, true)] = originalIndex;
// 				originalIndex++;
// 			}
// 			while (originalIndex < slotCount)
// 				unchanged[unchangedIndex++] = originalIndex++;
// 			for (ii = slotCount - 1; ii >= 0; ii--)
// 				if (drawOrder[ii] == -1) drawOrder[ii] = unchanged[--unchangedIndex];
// 			FREE(unchanged);
// 			spDrawOrderTimeline_setFrame(timeline, frameIndex, readFloat(self), drawOrder);
// 			FREE(drawOrder);
// 		}
// 		animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 		animation->duration = MAX(animation->duration, timeline->frames[n - 1]);
// 	}

// 	// Event timeline.
// 	n = readInt(self, true);
// 	if (n > 0) {

// 		spEventTimeline *timeline = spEventTimeline_create(n);
// 		int frameIndex;
// 		for (frameIndex = 0; frameIndex < n; frameIndex++) {

// 			spEvent *event;
// 			const char *stringValue;
// 			float time = readFloat(self);
// 			spEventData *eventData = skeletonData->events[readInt(self, true)];
// 			event = spEvent_create(eventData);
// 			event->intValue = readInt(self, false);
// 			event->floatValue = readFloat(self);
// 			stringValue = readBoolean(self) ? readString(self) : eventData->stringValue;
// 			if (stringValue) MALLOC_STR(event->stringValue, stringValue);
// 			spEventTimeline_setFrame(timeline, frameIndex, time, event);
// 		}
// 		animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
// 		animation->duration = MAX(animation->duration, timeline->frames[n - 1]);
// 	}

// 	skeletonData->animations[skeletonData->animationsCount++] = animation;
// }

// static spSkeletonData *readSkeleton(spSkeletonBinary *self)
// {
// 	int size, i;
// 	const char* buff;
// 	spSkeletonData *skeletonData;
// 	float scale = self->scale;

// 	FREE(self->json->error);
// 	CONST_CAST(char *, self->json->error) = 0;

// 	skeletonData = spSkeletonData_create();

// 	// Header
// 	if ((buff = readString(self)) != NULL) 
// 		MALLOC_STR(skeletonData->hash, buff);
// 	if ((buff = readString(self)) != NULL) 
// 		MALLOC_STR(skeletonData->version, buff);
// 	skeletonData->width = readFloat(self);
// 	skeletonData->height = readFloat(self);

// 	int nonessential = readBoolean(self);
// 	if (nonessential) {

// 		if ((buff = readString(self)) != NULL) {
// 			// TODO: imagesPath
// 		}
// 	}

// 	// Bones
// 	size = readInt(self, true);
// 	skeletonData->bones = MALLOC(spBoneData *, size);
// 	for (i = 0; i < size; i++) {

// 		spBoneData *parent = NULL;
// 		spBoneData *boneData;
// 		int parentIndex;

// 		const char *name = readString(self);
// 		parentIndex = readInt(self, true);
// 		if (parentIndex > -1) parent = skeletonData->bones[parentIndex];
// 		boneData = spBoneData_create(name, parent);
// 		boneData->x = readFloat(self) * scale;
// 		boneData->y = readFloat(self) * scale;
// 		boneData->scaleX = readFloat(self);
// 		boneData->scaleY = readFloat(self);
// 		boneData->rotation = readFloat(self);
// 		boneData->length = readFloat(self) * scale;
// 		boneData->flipX = readBoolean(self);
// 		boneData->flipY = readBoolean(self);
// 		boneData->inheritScale = readBoolean(self);
// 		boneData->inheritRotation = readBoolean(self);

// 		if (nonessential) {

// 			float r, g, b, a;
// 			readColor(self, &r, &g, &b, &a);
// 			// TODO: Color.rgba8888ToColor(boneData.color, input.readInt());
// 		}

// 		skeletonData->bones[i] = boneData;
// 		++skeletonData->bonesCount;
// 	}
	
// 	//ik
// 	size = readInt(self, true);
// 	skeletonData->ikConstraints = MALLOC(spIkConstraintData *, size);
// 	for (i = 0; i < size; i++) {

// 		int n;
// 		spIkConstraintData *ik = spIkConstraintData_create(readString(self));
// 		int boneCount = readInt(self, true);
// 		ik->bones = MALLOC(spBoneData *, boneCount);
// 		for (n = 0; n < boneCount; n++) {

// 			ik->bones[ik->bonesCount++] = skeletonData->bones[readInt(self, true)];
// 		}
// 		ik->target = skeletonData->bones[readInt(self, true)];
// 		ik->mix = readFloat(self);
// 		ik->bendDirection = readBoolean(self);
// 	}

// 	// Slots
// 	size = readInt(self, true);
// 	if (size > 0) {

// 		skeletonData->slots = MALLOC(spSlotData *, size);
// 		for (i = 0; i < size; i++) {
// 			const char *attachment;
// 			spBoneData *boneData;
// 			spSlotData *slotData;
		   
// 			const char *name = readString(self);
// 			boneData = skeletonData->bones[readInt(self, true)];
// 			slotData = spSlotData_create(name, boneData);
// 			readColor(self, &slotData->r, &slotData->g, &slotData->b, &slotData->a);
// 			attachment = readString(self);
// 			if (attachment) spSlotData_setAttachmentName(slotData, attachment);

// 			slotData->additiveBlending = readBoolean(self);

// 			skeletonData->slots[i] = slotData;
// 			++skeletonData->slotsCount;
// 		}
// 	}

// 	// Default skin.
// 	spSkin *defaultSkin = readSkin(self, "default", nonessential);
// 	if (defaultSkin != NULL)
// 		skeletonData->defaultSkin = defaultSkin;
// 	// Skins
// 	size = readInt(self, true);
// 	if (size <= 0 && defaultSkin != NULL) {

// 		// Add default skin
// 		skeletonData->skins = MALLOC(spSkin *, 1);
// 		skeletonData->skins[skeletonData->skinsCount] = defaultSkin;
// 		++skeletonData->skinsCount;
// 	} else if (size > 0) {

// 		// Add default skin
// 		skeletonData->skins = MALLOC(spSkin *, size + (defaultSkin == NULL) ? 0 : 1);
// 		if (defaultSkin != NULL) {

// 			skeletonData->skins[skeletonData->skinsCount] = defaultSkin;
// 			++skeletonData->skinsCount;
// 		}

// 		for (i = 0; i < size; i++) {

// 			const char *name = readString(self);
// 			spSkin *skin = readSkin(self, name, nonessential);
// 			skeletonData->skins[skeletonData->skinsCount] = skin;
// 			++skeletonData->skinsCount;
// 			if (strcmp("default", name) == 0)
// 				skeletonData->defaultSkin = skin;
// 		}
// 	}

// 	// Events
// 	size = readInt(self, true);
// 	if (size > 0) {
// 		const char *stringValue;
// 		skeletonData->events = MALLOC(spEventData *, size);
// 		for (i = 0; i < size; i++) {

// 			spEventData *eventData = spEventData_create(readString(self));
// 			eventData->intValue = readInt(self, false);
// 			eventData->floatValue = readFloat(self);
// 			stringValue = readString(self);
// 			if (stringValue) MALLOC_STR(eventData->stringValue, stringValue);
// 			skeletonData->events[skeletonData->eventsCount++] = eventData;
// 		}
// 	}

// 	// Animations
// 	size = readInt(self, true);
// 	if (size > 0) {

// 		skeletonData->animations = MALLOC(spAnimation *, size);
// 		for (i = 0; i < size; i++) {

// 			const char *name = readString(self);
// 			readAnimation(self, skeletonData, name);
// 		}
// 	}
// 	return skeletonData;
// }
// //
// //static void initStrs(spSkeletonBinary *self)
// //{
// //	int size, i;
// //	int skip = readInt(self, true);
// //	self->reader = self->rawdata + skip;
// //	size = readInt(self, true);
// //	self->strs = (char **)malloc(sizeof(char *) * size);
// //	for (i = 0; i < size; i++)
// //	{
// //		int len = readShort(self);
// //		self->strs[i] = self->reader;
// //		self->strs[i][len] = '\0';
// //		self->reader += len + 1;
// //	}
// //	
// //	self->reader = self->rawdata + 4;
// //}
// //
// spSkeletonData *spSkeletonBinary_readSkeletonData(const char *skeketonPath, spAtlas *atlas, float scale)
// {
// 	int length;
// 	spSkeletonData *skeketon;
// 	spSkeletonBinary *self;
// 	self = (spSkeletonBinary *)malloc(sizeof(spSkeletonBinary));
// 	self->scale = scale;
// 	self->json = spSkeletonJson_create(atlas);
// 	self->json->scale = scale;
// 	self->rawdata = _spUtil_readFile(skeketonPath, &length);
// 	self->reader = self->rawdata;
	
// //	initStrs(self);

// 	skeketon = readSkeleton(self);
// 	spSkeletonJson_dispose(self->json);
// //	free(self->strs);
// 	free(self->rawdata);
// 	free(self);
// 	return skeketon;
// }

spSkeletonData *spSkeletonBinary_readSkeletonData(const char *skeketonPath, spAtlas *atlas, float scale) {

	return NULL;
}
