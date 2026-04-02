#include "sandbox/layer.h"
#include "sandbox/inspectorRegistry.h"

#include "sandbox/dialog.h"

#include "engine/components/cameraComponent.h"
#include "engine/components/rendererComponent.h"
#include "engine/components/skyboxComponent.h"
#include "engine/components/directionalLightComponent.h"
#include "engine/components/pointLightComponent.h"
#include "engine/components/rigidbodyComponent.h"

#include <string>
#include <vector>

template <typename Setter>
static void DrawFilePicker(
	const char* label,
	const char* id,
	const std::string& currentPath,
	const std::vector<std::string>& filters,
	Setter setPath,
	bool showClearButton = true
) {
	ImGui::TextUnformatted(label);
	ImGui::SameLine();

	const float clearButtonWidth = ImGui::GetFrameHeight();
	const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	float chooseButtonWidth = ImGui::GetContentRegionAvail().x - clearButtonWidth - spacing;
	if (chooseButtonWidth < 80.0f) {
		chooseButtonWidth = 80.0f;
	}

	const std::string chooseText = currentPath.empty() ? "Choose file..." : currentPath;
	const std::string chooseLabel = chooseText + "##choose_" + id;
	if (ImGui::Button(chooseLabel.c_str(), ImVec2(chooseButtonWidth, 0.0f))) {
		std::string chosenPath = Dialog::openFile(filters);
		if (!chosenPath.empty()) {
			setPath(chosenPath);
		}
	}

	if (!showClearButton) return;

	ImGui::SameLine(0.0f, spacing);
	if (currentPath.empty()) {
		ImGui::BeginDisabled();
	}
	const std::string clearLabel = std::string("X##clear_") + id;
	if (ImGui::Button(clearLabel.c_str(), ImVec2(clearButtonWidth, 0.0f))) {
		setPath("");
	}
	if (currentPath.empty()) {
		ImGui::EndDisabled();
	}
}

void Layer::registerDefaultInspectors() {
	InspectorRegistry::registerComponent<CameraComponent>([](CameraComponent* c) {
		float fov = c->getCamera().getFov();
		if (ImGui::DragFloat("FOV", &fov, 0.5f, 10.0f, 170.0f))
			c->getCamera().setFov(fov);
		float nearFar[2] = { c->getCamera().getNear(), c->getCamera().getFar() };
		if (ImGui::DragFloat2("Near/Far", nearFar, 0.1f, 0.01f, 10000.0f, "Near: %.2f\nFar: %.2f")) {
			if (nearFar[1] <= nearFar[0]) {
				nearFar[1] = nearFar[0] + 0.01f;
			}
			c->getCamera().setNear(nearFar[0]);
			c->getCamera().setFar(nearFar[1]);
		}
	});

	InspectorRegistry::registerComponent<RenderComponent>([](RenderComponent* c) {
		DrawFilePicker(
			"Model",
			"model_path",
			c->getModelPath(),
			{ "Model Files", "*.obj *.fbx *.dae *.gltf *.glb", "All Files", "*" },
			[&](const std::string& path) { c->setModelPath(path); }
		);

		ImGui::SeparatorText("Shader");
		DrawFilePicker(
			"Vertex Shader",
			"vert_path",
			c->getVertPath(),
			{ "Shader Files", "*.glsl *.vert *.vs", "All Files", "*" },
			[&](const std::string& path) { c->setVertPath(path); }
		);

		DrawFilePicker(
			"Fragment Shader",
			"frag_path",
			c->getFragPath(),
			{ "Shader Files", "*.glsl *.frag *.fs", "All Files", "*" },
			[&](const std::string& path) { c->setFragPath(path); }
		);

		ImGui::SeparatorText("Textures");
		DrawFilePicker(
			"Diffuse Texture",
			"diffuse_path",
			c->getDiffuseTexturePath(),
			{ "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
			[&](const std::string& path) { c->setDiffuseTexturePath(path); }
		);

		DrawFilePicker(
			"Specular Texture",
			"specular_path",
			c->getSpecularTexturePath(),
			{ "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
			[&](const std::string& path) { c->setSpecularTexturePath(path); }
		);

		DrawFilePicker(
			"Normal Texture",
			"normal_path",
			c->getNormalTexturePath(),
			{ "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
			[&](const std::string& path) { c->setNormalTexturePath(path); }
		);

		DrawFilePicker(
			"Metallic Texture",
			"metallic_path",
			c->getMetallicTexturePath(),
			{ "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
			[&](const std::string& path) { c->setMetallicTexturePath(path); }
		);

		ImGui::SeparatorText("Material");
		Vec3 baseColor = c->getBaseColor();
		if (ImGui::ColorEdit3("Base Color", &baseColor.x)) {
			c->setBaseColor(baseColor);
		}

		float ambientStrength = c->getAmbientStrength();
		if (ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f, 0.0f, 2.0f)) {
			c->setAmbientStrength(ambientStrength);
		}

		float specularStrength = c->getSpecularStrength();
		if (ImGui::DragFloat("Specular Strength", &specularStrength, 0.05f, 0.0f, 8.0f)) {
			c->setSpecularStrength(specularStrength);
		}
        
		float shininess = c->getShininess();
		if (ImGui::DragFloat("Shininess", &shininess, 1.0f, 1.0f, 256.0f)) {
			c->setShininess(shininess);
		}

		Vec2 uvScale = c->getUVScale();
		float uv[2] = { uvScale.x, uvScale.y };
		if (ImGui::DragFloat2("UV Scale", uv, 0.1f, 0.01f, 128.0f)) {
			c->setUVScale(Vec2(uv[0], uv[1]));
		}
	});

	InspectorRegistry::registerComponent<SkyboxComponent>([](SkyboxComponent* c) {
		std::vector<std::string> faces = c->getFaces();
		const char* faceLabels[6] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };
		const char* faceIds[6] = { "px", "nx", "py", "ny", "pz", "nz" };

		int moveFrom = -1;
		int moveTo = -1;

		ImGui::SeparatorText("Cubemap Faces");
		
		for (size_t i = 0; i < faces.size() && i < 6; ++i) {
			ImGui::PushID(static_cast<int>(i));
			
			ImGui::BeginGroup();
			
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::TextUnformatted(":::"); 
			ImGui::PopStyleColor();
			
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				int sourceIdx = static_cast<int>(i);
				ImGui::SetDragDropPayload("DND_SKYBOX_FACE", &sourceIdx, sizeof(int));
				ImGui::Text("Moving %s", faceLabels[i]);
				ImGui::EndDragDropSource();
			}

			ImGui::SameLine();

			const std::string pickerId = std::string("##face_") + faceIds[i];
			DrawFilePicker(
				faceLabels[i],
				pickerId.c_str(),
				faces[i],
				{ "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
				[&, i](const std::string& path) { c->setFacePath((size_t)i, path); },
				false // no clear button
			);
			
			ImGui::EndGroup();

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SKYBOX_FACE")) {
					moveFrom = *(const int*)payload->Data;
					moveTo = static_cast<int>(i);
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::PopID();
		}

		if (moveFrom != -1 && moveTo != -1 && moveFrom != moveTo) {
			std::string moved = faces[moveFrom];
			faces.erase(faces.begin() + moveFrom);
			faces.insert(faces.begin() + moveTo, moved);
			c->setFaces(faces);
		}

		ImGui::Separator();
		float rotation = c->getRotation();
		if (ImGui::DragFloat("Rotation", &rotation, 0.1f, 0.0f, 360.0f)) {
			c->setRotation(rotation);
		}
	});

	InspectorRegistry::registerComponent<PointLightComponent>([](PointLightComponent* c) {
		Vec3 color = c->getColor();
		if (ImGui::ColorEdit3("Color", &color.x)) {
			c->setColor(color);
		}

		float intensity = c->getIntensity();
		if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f)) {
			c->setIntensity(intensity);
		}

		float radius = c->getRadius();
		if (ImGui::DragFloat("Radius", &radius, 1.0f, 0.0f, 2500.0f)) {
			c->setRadius(radius);
		}
	});

	InspectorRegistry::registerComponent<DirectionalLightComponent>([](DirectionalLightComponent* c) {
		Vec3 direction = c->getDirection();
		if (ImGui::DragFloat3("Direction", &direction.x, 0.1f, -1.0f, 1.0f)) {
			c->setDirection(direction.normalize());
		}

		Vec3 color = c->getColor();
		if (ImGui::ColorEdit3("Color", &color.x)) {
			c->setColor(color);
		}

		float intensity = c->getIntensity();
		if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f)) {
			c->setIntensity(intensity);
		}
	});

	InspectorRegistry::registerComponent<RigidbodyComponent>([this](RigidbodyComponent* c) {
        bool useGravity = c->getUseGravity();
        if (ImGui::Checkbox("Use Gravity", &useGravity)) {
            c->setUseGravity(useGravity);
        }

		const char* bodyTypes[] = { "Dynamic", "Static" };
		int currentBodyType = static_cast<int>(c->getBodyType());
		if (ImGui::Combo("Body Type", &currentBodyType, bodyTypes, IM_ARRAYSIZE(bodyTypes))) {
			c->setBodyType(static_cast<RigidbodyComponent::BodyType>(currentBodyType));
		}

        ImGui::SeparatorText("Collider");
        Vec3 colliderSize = c->getColliderSize();
        if (ImGui::DragFloat3("Collider Size", &colliderSize.x, 0.1f, 0.01f, 1000.0f)) {
            c->setColliderSize(colliderSize);
        }

        const char* colliderTypes[] = { "Box", "Sphere", "Capsule" };
        int currentColliderType = static_cast<int>(c->getColliderType());
        if (ImGui::Combo("Collider Type", &currentColliderType, colliderTypes, IM_ARRAYSIZE(colliderTypes))) {
            c->setColliderType(static_cast<RigidbodyComponent::ColliderType>(currentColliderType));
        }

		const bool currentlyVisible = c->entity && m_ColliderDebugEntities.find(c->entity) != m_ColliderDebugEntities.end();
		bool showCollider = currentlyVisible;
		if (ImGui::Checkbox("Show Collider", &showCollider) && c->entity) {
			if (showCollider) {
				m_ColliderDebugEntities.insert(c->entity);
			} else {
				m_ColliderDebugEntities.erase(c->entity);
			}
		}

        ImGui::SeparatorText("Properties");
        float mass = c->getMass();
        if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 1000.0f)) {
            c->setMass(mass);
        }

        float friction = c->getFriction();
        if (ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 1.0f)) {
            c->setFriction(friction);
        }   

        float restitution = c->getRestitution();
        if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f)) {
            c->setRestitution(restitution);
        }

        float linearDamping = c->getLinearDamping();
        if (ImGui::DragFloat("Linear Damping", &linearDamping, 0.01f, 0.0f, 1.0f)) {
            c->setLinearDamping(linearDamping);
        }

        float angularDamping = c->getAngularDamping();
        if (ImGui::DragFloat("Angular Damping", &angularDamping, 0.01f, 0.0f, 1.0f)) {
            c->setAngularDamping(angularDamping);
        }
    });
}