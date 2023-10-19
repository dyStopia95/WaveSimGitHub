#include "pch.h"
#include "NodeArray.h"
#include "GameTime.h"

namespace Rendering
{
	int NodeArray::GetIndex(int row, int column)
	{
		return { row * _columns + column };
	}

	Rendering::Node& NodeArray::GetNode(int row, int column)
	{
		//Do not need to check if both row and column are < 0 / = max row/column as these nodes can never be requested for finding curvature

		if (row < 0)
		{
			return _nodeArray[GetIndex(0, column)];
		}
		if (row == _rows)
		{
			return _nodeArray[GetIndex(row - 1, column)];
		}
		if (column < 0)
		{
			return _nodeArray[GetIndex(row, 0)];
		}
		if (column == _columns)
		{
			return _nodeArray[GetIndex(row, column - 1)];
		}

		return _nodeArray[GetIndex(row, column)];
	}

	float NodeArray::GetNodeDisplacement(int row, int column)
	{
		return GetNode(row, column)._displacement;
	}

	float NodeArray::GetNodeVelocity(int row, int column)
	{
		return GetNode(row, column)._velocity;
	}

	float NodeArray::GetAcceleration(int i, int j)
	{
		const Node& node = GetNode(i, j);
		float curvature =
			(GetNodeDisplacement(i + 1, j) + GetNodeDisplacement(i - 1, j) + GetNodeDisplacement(i, j + 1) + GetNodeDisplacement(i, j - 1) - 4 * node._displacement) / H2;

		return ((C2 * curvature) - (_dampingFactor * node._velocity) - (_k * node._displacement));
		//return (-(C2 * curvature) / 75);
	}

	void NodeArray::Initialize()
	{
		C2 = _C * _C;
		_nodeCount = _rows * _columns;
		_nodeArray.reserve(_nodeCount);
		for (int i = 0; i < _nodeCount; ++i)
			_nodeArray.push_back(Node{});

		int ID = 0;
		for (int i = 0; i < _rows; ++i)
		{
			for (int j = 0; j < _columns; ++j)
			{
				Node& node = GetNode(i, j);

				node._displacement = 0;
				node._isForced = false;
				node._position = DirectX::XMFLOAT2{ i * _nodeSpacing,j * _nodeSpacing };
				node._nodeID = ID;
				++ID;
			}
		}

		_nodeArray[5049]._velocity = _node0InitialV;
	}

	void NodeArray::Update(const Library::GameTime& gameTime)
	{
		float dT = gameTime.ElapsedGameTimeSeconds().count();
		dT *= 12;
		for (int i = 0; i < _rows; ++i)
		{
			for (int j = 0; j < _columns; ++j)
			{
				Node& node = GetNode(i, j);
				float a = GetAcceleration(i, j);
				//node.UpdateNode(a, dT);
				node.UpdateNode(a, _deltaT);
			}
		}
	}

	void NodeArray::SetDampingFactor(float dmpFactor)
	{
		_dampingFactor = dmpFactor;
	}

	void NodeArray::SetSpringConstant(float springK)
	{
		_k = springK;
	}

	void NodeArray::SetNode0InitialVel(float initVel)
	{
		_node0InitialV = initVel;
	}

	void NodeArray::SetTimeStep(float deltaT)
	{
		_deltaT = deltaT;
	}

	void NodeArray::SetRowColumn(int rows, int columns)
	{
		_rows = rows;
		_columns = columns;
	}

	void NodeArray::SetNodeSpacing(float spacing)
	{
		_nodeSpacing = spacing;
		H2 = _nodeSpacing * _nodeSpacing;
	}

	void NodeArray::SetWaveConstantC(float c)
	{
		_C = c;
		C2 = _C * _C;
	}

	void NodeArray::SetBulkVariables(int rows, int columns, float spacing, float c, float deltaT, float initVel, float dmpFactor)
	{
		SetRowColumn(rows, columns);
		SetNodeSpacing(spacing);
		SetWaveConstantC(c);
		SetTimeStep(deltaT);
		SetNode0InitialVel(initVel);
		SetDampingFactor(dmpFactor);
	}

	void NodeArray::SetBulkVariables(SimParams& params)
	{
		SetRowColumn(params.rows, params.columns);
		SetNodeSpacing(params.spacing);
		SetWaveConstantC(params.c);
		SetTimeStep(params.deltaT);
		SetNode0InitialVel(params.initVel);
		SetDampingFactor(params.dmpFactor);
		SetSpringConstant(params.k);
	}

	SimParams::SimParams(int r, int c, float s, float k, float dT, float iV, float dmpF, float springK) :
		rows{ r }, columns{ c }, spacing{ s }, c{ k }, deltaT{ dT }, initVel{ iV }, dmpFactor{ dmpF }, k{ springK }
	{
	}
}