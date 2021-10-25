add_library(OverlordEngine 
	#base
	GameTime.cpp
	InputManager.cpp
	MaterialManager.cpp
	SoundManager.cpp
	OverlordGame.cpp
	#Components
	BaseComponent.cpp
	CameraComponent.cpp
	ColliderComponent.cpp
	MeshDrawComponent.cpp
	ControllerComponent.cpp
	MeshIndexedDrawComponent.cpp
	ParticleEmitterComponent.cpp
	RigidBodyComponent.cpp
	ModelComponent.cpp
	SelfDestructionComponent.cpp
	SpriteComponent.cpp
	TransformComponent.cpp
	#Content
	PxTriangleMeshLoader.cpp
	PxConvexMeshLoader.cpp
	ContentManager.cpp
	EffectLoader.cpp
	TextureDataLoader.cpp
	MeshFilterLoader.cpp
	SpriteFontLoader.cpp
	#Diagnostics
	DebugRenderer.cpp
	GameSpecs.cpp
	Logger.cpp


	#Graphics
	RenderTarget.cpp
	TextureData.cpp
	TextRenderer.cpp
	SpriteRenderer.cpp
	ShadowMapRenderer.cpp
	Particle.cpp
	ModelAnimator.cpp
	MeshFilter.cpp
	Material.cpp
	UberMaterial.cpp
	SpikeyMaterial.cpp
	SkinnedDiffuseMaterial.cpp
	DiffuseMaterial.cpp
	ColorMaterial.cpp
	SkinnedDiffuseMaterial_Shadow.cpp
	ShadowMapMaterial.cpp
	DiffuseMaterial_Shadow.cpp
	CenterGlow.cpp
	CombineResources.cpp
	PostBlur.cpp
	PostGrayscale.cpp
	PostProcessingMaterial.cpp

	#Helpers
	BinaryReader.cpp
	EffectHelper.cpp
	PhysXHelper.cpp
	VertexHelper.cpp
	SpriteFont.cpp

	#PhysX
	PhysXErrorCallback.cpp
	PhysXManager.cpp
	PhysXProxy.cpp

	#Prefabs
	CubePrefab.cpp
	FixedCamera.cpp
	FreeCamera.cpp
	SpherePrefab.cpp
	TorusPrefab.cpp
	BoneObject.cpp

	#SceneGraph

	GameObject.cpp
	GameScene.cpp
	SceneManager.cpp 

	stdafx.cpp
	)



target_include_directories (PortalGame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DUNICODE)