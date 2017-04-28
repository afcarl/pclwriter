/*
 * ypclwriter.h
 *
 *  Created on:
 *      Author: Chunfeng Yang
 *
 *  Version 	0.3.0.1
 *        save the grid as the current grid ID;
 *
 *  Version 	0.5.0.1
 *         in saveGrid, XZ plane -> ZX plane
 *
 *  Version 	0.5.1.1
 *         adding clear()
 *
 *  Version 	0.6.0.1
 *         adding class DualCurvePlane
 */

#ifndef NODE_INC_YPCLWRITER_H_
#define NODE_INC_YPCLWRITER_H_
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <map>
#include <algorithm>
#include <iterator>

#include <sstream>
#include <node.h>
#include <node_object_wrap.h>

#define SSTR( x ) static_cast< std::ostringstream & >(\
		( std::ostringstream() << std::dec << x ) ).str()

class Grid
{
public:
	Grid()
	{
		_ID = -1;
		_CoordID = 0;
	}
	;
	virtual ~Grid()
	{
	}
	;
public:
	bool isSame(Grid theGrid, double error)
	{
		bool result = false;
		if (fabs(_X - theGrid._X) > error)
		{
			return result;
		}
		if (fabs(_Y - theGrid._Y) > error)
		{
			return result;
		}
		if (fabs(_Z - theGrid._Z) > error)
		{
			return result;
		}
		if (_CoordID != theGrid._CoordID)
		{
			return result;
		}
		result = true;
		return result;
	}
public:
	int _ID;
	int _CoordID;
	double _X;
	double _Y;
	double _Z;

};

class Curve
{
public:
	Curve()
	{
		_ID = -1;
	}
	;
	virtual ~Curve()
	{
	}
	;
public:
	bool isSame(Curve theCurve)
	{
		bool result = false;

		if ((_startGridID == theCurve._startGridID)
				&& (_endGridID == theCurve._endGridID))
		{
			result = true;
		}

		if ((_endGridID == theCurve._startGridID)
				&& (_startGridID == theCurve._endGridID))
		{
			result = true;
		}
		return result;
	}
public:
	int _ID;
	int _startGridID;
	int _endGridID;
};

class DualCurvePlane
{
public:
	DualCurvePlane();
	virtual ~DualCurvePlane();

public:
	bool isSame(DualCurvePlane thePlane)
	{
		bool result = false;

		if ((_startCurveID == thePlane._startCurveID)
				&& (_endCurveID == thePlane._endCurveID))
		{
			result = true;
		}

		if ((_endCurveID == thePlane._startCurveID)
				&& (_startCurveID == thePlane._endCurveID))
		{
			result = true;
		}
		return result;
	}
private:
	int _ID;
	int _startCurveID;
	int _endCurveID;
};

class YPCLWriter: public node::ObjectWrap
{
public:
	static void Init(v8::Local<v8::Object> exports);

private:
	explicit YPCLWriter();
	virtual ~YPCLWriter();

	static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	static v8::Persistent<v8::Function> constructor;

	static void open(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void close(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void clear(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setOutputFilePath( const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setPCLCommandFilePath( const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setSectionPCLCommandMap(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void writePCL(const v8::FunctionCallbackInfo<v8::Value>& args);
	//
	//  svg path parser
	//
	static void writeSesFileHeader(
			const v8::FunctionCallbackInfo<v8::Value>& args);
	static void parserSVGLine(const v8::FunctionCallbackInfo<v8::Value>& args);

	static void write(const v8::FunctionCallbackInfo<v8::Value>& args);

	//
	// properties
	//
	static void setCoordID(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setCoordPlaneID(
			const v8::FunctionCallbackInfo<v8::Value>& args);

	static void getCoordID(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getCoordPlaneID(
			const v8::FunctionCallbackInfo<v8::Value>& args);

	//
	// Specification
	//
	static void createGrid(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getGrid(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void createCurve(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getCurve(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void constructBiCurveSurface(
			const v8::FunctionCallbackInfo<v8::Value>& args);

	static void setBaseCenter(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getBaseCenter(const v8::FunctionCallbackInfo<v8::Value>& args);

	static void getGridID(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setGridID(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void getCurveID(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setCurveID(const v8::FunctionCallbackInfo<v8::Value>& args);

	static void getBiCurveSurfaceID(
			const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setBiCurveSurfaceID(
			const v8::FunctionCallbackInfo<v8::Value>& args);

	static void getStartID(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void setStartID(const v8::FunctionCallbackInfo<v8::Value>& args);

public:
	void clear();

private:
	void parserPCLCommandLine(char *buffer);
	std::vector<std::string> parserLine(char *buffer);
	void writePCLCommand(FILE *outfp, std::string name,
			const std::vector<std::string> &vec);
	void sesHeader(FILE *outfp, char* theCurrentDir, char* theJobName);

public:
	char* fgets(char *s, int n, FILE *iop);
	int fputs(char *s, FILE *iop);
	int getline(char *line, int max, FILE *fp);
	size_t trimwhitespace(char *out, size_t len, const char *str);

	//
	//  svg path parser
	//

private:
	char *_version;

	char* _inputFilePath;
	char* _outputFilePath;
	char* _pclCommandFilePath;

	std::map<std::string, std::vector<std::string>> _pclComandMap;

	std::map<std::string, std::string> _sectionPCLComandMap;

	FILE *_outfp;

	//
	//  svg path parser
	//
	Grid getGrid(int id);
	Curve getCurve(int id);
	int saveArc(int startGridID, int endGridID, double radius, double rotation,
			int clockwise, int arc_angle);
	int saveGrid(double x, double y);
	int saveGrid(double x, double y, double z);
	int saveLine(int startGridID, int endGridID);
	int saveBiCurveSurface(int startID, int endID);
	void parserSVGLine(char *buffer);

	double section_A_Z = 0.0;
	int _coordID = 0;
	int _coordPlaneID = 1;
	int GRID_ID = 0;
	int LINE_ID = 0;
	int DUAL_CURVE_SURFACE_ID = 0;
	int _StartID = 0;
	std::vector<Grid> gridPool;
	std::vector<Curve> curvePool;
	std::vector<Curve> biCurveSurfacePool;

	std::vector<int> gridVector;
	std::vector<int> curveVector;
	std::vector<int> dul_curve_surfaceVector;

	double _SectionBaseCenter_X = 0.0;
	double _SectionBaseCenter_Y = 0.0;
	double _SectionBaseCenter_Z = 0.0;

	double _Base_Grid_X = 0.0;
	double _Base_Grid_Y = 0.0;
	double _Base_Grid_Z = 0.0;

	double _current_x;
	double _current_y;
	double _current_z;

	int _currentGridID = 0;
	int _currentCurveID = 0;
	int _currentDualCurveSurfaceID = 0;

	double _globalCartesianCoordOrign_X = 0.0;
	double _globalCartesianCoordOrign_Y = 0.0;
	double _globalCartesianCoordOrign_Z = 0.0;

	double _localCartesianCoordOrign_X = 0.0;
	double _localCartesianCoordOrign_Y = 0.0;
	double _localCartesianCoordOrign_Z = 0.0;
};

#endif /* NODE_INC_YPCLWRITER_H_ */
