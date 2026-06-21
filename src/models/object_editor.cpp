#include "object_editor.h"

#include <glm/vec3.hpp>

GLuint Object_Editor::resolveTexture(GLuint planetTexture, GLuint poopTexture, GLuint sunTexture) const
{
	GLuint textureId = planetTexture;
	if (textureChoice_ == 1) {
		textureId = poopTexture;
	} else if (textureChoice_ == 2) {
		textureId = sunTexture;
	}
	return textureId;
}

GLuint Object_Editor::createPreviewTexture() const
{
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	const unsigned char whitePixel[4] = {255, 255, 255, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return textureId;
}

void Object_Editor::toggleOpen()
{
	open_ = !open_;
	if (!open_) {
		clearPreview();
	}
}

glm::vec3 Object_Editor::resolveSpawnPosition(const Plane* plane) const
{
	glm::vec3 spawnPos(position_[0], position_[1], position_[2]);
	if (plane == nullptr) {
		return spawnPos;
	}

	const float safetyMargin = 1.0f;
	for (const auto& existing : plane->objs) {
		if (!existing || existing->IsPreview) {
			continue;
		}

		float dx = spawnPos.x - existing->pos.x;
		float dy = spawnPos.y - existing->pos.y;
		float dz = spawnPos.z - existing->pos.z;
		float dist2 = dx * dx + dy * dy + dz * dz;
		float minDist = existing->radius + radius_ + safetyMargin;
		if (dist2 < minDist * minDist) {
			spawnPos.x = existing->pos.x + minDist;
		}
	}

	return spawnPos;
}

void Object_Editor::syncPreview(const sphere& mesh, Plane* plane, GLuint planetTexture, GLuint poopTexture, GLuint sunTexture)
{
	if (plane == nullptr) {
		return;
	}

	glm::vec3 spawnPos = resolveSpawnPosition(plane);
	if (previewTexture_ == 0) {
		previewTexture_ = createPreviewTexture();
	}

	previewObject_ = Object(
		velocity_[0], velocity_[1], velocity_[2],
		spawnPos,
		mesh.VAO,
		mesh.VBO,
		mesh.EBO,
		mass_,
		mesh.indexCount,
		radius_,
		previewTexture_,
		isBlackHole_,
		isLightSource_);
	float previewScale = radius_ * 1000.0f;
	if (previewScale < 1.0f) {
		previewScale = 1.0f;
	}
	previewObject_.modelMatrix = glm::translate(glm::mat4(1.0f), spawnPos);
	previewObject_.modelMatrix = glm::scale(previewObject_.modelMatrix, glm::vec3(previewScale));
	previewObject_.IsPreview = false;
	previewVisible_ = true;
}

void Object_Editor::clearPreview()
{
	previewVisible_ = false;
	previewObject_ = Object();
	if (previewTexture_ != 0) {
		glDeleteTextures(1, &previewTexture_);
		previewTexture_ = 0;
	}
}

Object_Editor::Object_Editor()
{

}
Object_Editor::~Object_Editor()
{
    clearPreview();
}

void Object_Editor::init(GLFWwindow* window, const char* glsl_version)
{
	(void)window;
	(void)glsl_version;
}

void Object_Editor::shutdown()
{
	clearPreview();
}

void Object_Editor::beginFrame()
{
}

bool Object_Editor::render(int windowWidth, int windowHeight, Plane* plane,
						   const sphere& mesh,
						   GLuint planetTexture,
						   GLuint poopTexture,
						   GLuint sunTexture,
						   glm::vec3* cameraPosition,
						   bool* firstMouse)
{
	(void)windowWidth;
	(void)windowHeight;

	if (!open_) {
		return false;
	}

	ImGui::Begin("Object Editor", &open_);

	ImGui::Text("Spawn a new object into the simulation");
	ImGui::InputFloat3("Position", position_);
	ImGui::InputFloat3("Velocity", velocity_);
	ImGui::DragFloat("Mass", &mass_, 0.1f, 0.0f, 1.0e12f, "%.3f");
	ImGui::DragFloat("Radius", &radius_, 0.1f, 0.01f, 1.0e6f, "%.3f");

	const char* textureItems[] = {"Planet", "Poop", "Sun"};
	ImGui::Combo("Texture", &textureChoice_, textureItems, IM_ARRAYSIZE(textureItems));
	ImGui::Checkbox("Black hole", &isBlackHole_);
	ImGui::Checkbox("Light source", &isLightSource_);

	if (previewVisible_) {
		syncPreview(mesh, plane, planetTexture, poopTexture, sunTexture);
	}

	if (ImGui::Button("Add Object")) {
		clearPreview();
		GLuint textureId = resolveTexture(planetTexture, poopTexture, sunTexture);

		glm::vec3 spawnPos = resolveSpawnPosition(plane);
		position_[0] = spawnPos.x;
		position_[1] = spawnPos.y;
		position_[2] = spawnPos.z;

		plane->objs.push_back(new Object(
			velocity_[0], velocity_[1], velocity_[2],
			spawnPos,
			mesh.VAO,
			mesh.VBO,
			mesh.EBO,
			mass_,
			mesh.indexCount,
			radius_,
			textureId,
			isBlackHole_,
			isLightSource_));

		if (cameraPosition != nullptr) {
			*cameraPosition = spawnPos;
		}
		if (firstMouse != nullptr) {
			*firstMouse = true;
		}
		clearPreview();
	}

	ImGui::SameLine();
	if (ImGui::Button("Reset Draft")) {
		position_[0] = position_[1] = position_[2] = 0.0f;
		velocity_[0] = velocity_[1] = velocity_[2] = 0.0f;
		mass_ = 1.0f;
		radius_ = 1.0f;
		textureChoice_ = 0;
		isBlackHole_ = false;
		isLightSource_ = false;
		if (previewVisible_) {
			syncPreview(mesh, plane, planetTexture, poopTexture, sunTexture);
		}
	}
    ImGui::SameLine();
    if(ImGui::Button("Port to new position")){
        if (cameraPosition != nullptr) {
            *cameraPosition = glm::vec3(position_[0], position_[1], position_[2]);
        }
    }
    ImGui::Spacing();
    if(ImGui::Button("Add a new Texture"))
	{
		// Implementation for adding a new texture
	}
	ImGui::SameLine();
	if(ImGui::Button("Draw Preview"))
	{
		syncPreview(mesh, plane, planetTexture, poopTexture, sunTexture);
	}
	ImGui::End();
	return true;
}

void Object_Editor::drawPreview(const class shader& objectShader, glm::mat4 projection, glm::mat4 view) const
{
	if (!open_ || !previewVisible_) {
		return;
	}

	GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	previewObject_.draw(objectShader, projection, view);
	if (depthWasEnabled) {
		glEnable(GL_DEPTH_TEST);
	}
}