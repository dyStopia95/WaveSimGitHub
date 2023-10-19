#pragma once
#include <vector>
#include "DirectXMath.h"
#include "VectorHelper.h"
#include "GameClock.h"

namespace Rendering
{
	struct SimParams
	{
		int rows{0};
		int columns{0};
		float spacing{0};
		float c{0};
		float deltaT{0};
		float initVel{0};
		float dmpFactor{0};
		float k{ 0 };

		SimParams() = default;
		SimParams(int r, int c, float s, float k, float dT, float iV, float dmpF, float springK);
	};

	struct Node
	{
		DirectX::XMFLOAT2 _position = Library::Vector2Helper::Zero;
		float _displacement = 0.f;
		float _velocity = 0.f;
		bool _isForced = false;
		int _nodeID = 0;

		float UpdateVelocity(float acceleration, float DeltaTime) {
			_velocity += acceleration * DeltaTime;
			return _velocity;
		}
		float UpdateDisplacement(float velocity, float DeltaTime) {
			_displacement += velocity * DeltaTime;
			return _displacement;
		}
		float UpdateNode(float acceleration, float DeltaTime) {
			float v = UpdateVelocity(acceleration, DeltaTime);
			float d = UpdateDisplacement(v, DeltaTime);
			return d;
		}
		float UpdateNode(float forcedDisplacement) {
			_displacement = forcedDisplacement;
			return _displacement;
		}
	};

	class NodeArray final
	{
		std::vector<Rendering::Node> _nodeArray;

		//User provided
		float _nodeSpacing{ 10.f };
		int _rows{ 10 };
		int _columns{ 10 };
		float _C{ 3.f };
		float _dampingFactor{ 0.f };
		float _node0InitialV{ 1.f };
		float _deltaT = 1.f;
		float _k{ 0 };

		//Derived
		float C2{ 0.f };
		float H2{ 0.f };
		int _nodeCount{ 0 };
		float _avgDisplacement{ 0.f };

		void SetRowColumn(int rows, int columns);
		void SetNodeSpacing(float spacing);

	public:
		//Variable setting functions
		void SetWaveConstantC(float c);
		void SetTimeStep(float deltaT);
		void SetNode0InitialVel(float initVel);
		void SetDampingFactor(float dmpFactor);
		void SetSpringConstant(float springK);
		void SetBulkVariables(
			int rows, int columns,
			float spacing,
			float c,
			float deltaT,
			float initVel,
			float dmpFactor
		);
		void SetBulkVariables(SimParams& params);

	private:
		//Simulation functions
		int GetIndex(int row, int column);
		Rendering::Node& GetNode(int row, int column);
		float GetNodeDisplacement(int row, int column);
		float GetNodeVelocity(int row, int column);
		float GetAcceleration(int i, int j);

	public:
		void Initialize();
		void Update(const Library::GameTime& gameTime);
		int GetNodeCount() { return _nodeCount; };
		int GetRows() { return _rows; };
		int GetColumns() { return _columns; };

		std::vector<Rendering::Node>& GetArray() { return _nodeArray; };


	};


}
