/*
 * ypclwriter.cpp
 *
 *  Created on: 2016年10月20日
 *      Author: Chunfeng Yang
 *
 *
 */

#include "ypclwriter.h"

using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Array;

Persistent<Function> YPCLWriter::constructor;
YPCLWriter::YPCLWriter()
{
	_version = "0.6.0.1";

	_inputFilePath = NULL;
	_outputFilePath = NULL;

	_outfp = NULL;

	_current_x = 0.0;
	_current_y = 0.0;

}

YPCLWriter::~YPCLWriter()
{
	clear();
	_pclComandMap.clear();
	_sectionPCLComandMap.clear();
	if (NULL != _outfp)
	{
		fclose(_outfp);
		_outfp = NULL;
	}
}

void YPCLWriter::Init(Local<Object> exports)
{
	Isolate* isolate = exports->GetIsolate();

	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "YPCLWriter"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	//NODE_SET_PROTOTYPE_METHOD(tpl, "version", version);

	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "open", open);
	NODE_SET_PROTOTYPE_METHOD(tpl, "close", close);
	NODE_SET_PROTOTYPE_METHOD(tpl, "clear", clear);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setOutputFilePath", setOutputFilePath);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setPCLCommandFilePath", setPCLCommandFilePath);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setSectionPCLCommandMap", setSectionPCLCommandMap);
	NODE_SET_PROTOTYPE_METHOD(tpl, "writePCL", writePCL);
	NODE_SET_PROTOTYPE_METHOD(tpl, "write", write);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getCoordID", getCoordID);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setCoordID", setCoordID);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getCoordPlaneID", getCoordPlaneID);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setCoordPlaneID", setCoordPlaneID);

	NODE_SET_PROTOTYPE_METHOD(tpl, "parserSVGLine", parserSVGLine);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getBaseCenter", getBaseCenter);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setBaseCenter", setBaseCenter);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getGridID", getGridID);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setGridID", setGridID);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getCurveID", getCurveID);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setCurveID", setCurveID);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getBiCurveSurfaceID", getBiCurveSurfaceID);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setBiCurveSurfaceID", setBiCurveSurfaceID);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getStartID", getStartID);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setStartID", setStartID);

	NODE_SET_PROTOTYPE_METHOD(tpl, "constructGrid", createGrid);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getGrid", getGrid);

	NODE_SET_PROTOTYPE_METHOD(tpl, "constructCurve", createCurve);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getCurve", getCurve);

	NODE_SET_PROTOTYPE_METHOD(tpl, "constructBiCurveSurface", constructBiCurveSurface);

	NODE_SET_PROTOTYPE_METHOD(tpl, "writeSesFileHeader", writeSesFileHeader);

	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "YPCLWriter"),
			tpl->GetFunction());
}

void YPCLWriter::New(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall())
	{
		// Invoked as constructor: `new YPCLWriter(...)`
		//double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
		YPCLWriter* obj = new YPCLWriter();
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else
	{
		// Invoked as plain function `YPCLWriter(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] =
		{ args[0] };
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
	return;
}

void YPCLWriter::clear(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	obj->clear();

	return;
}
void YPCLWriter::clear()
{
	section_A_Z = 0.0;
	_coordID = 0;
	_coordPlaneID = 1;
	GRID_ID = 0;
	LINE_ID = 0;
	DUAL_CURVE_SURFACE_ID = 0;
	_StartID = 0;

	_current_x = 0.0;
	_current_y = 0.0;

	_SectionBaseCenter_X = 0.0;
	_SectionBaseCenter_Y = 0.0;
	_SectionBaseCenter_Z = 0.0;

	_Base_Grid_X = 0.0;
	_Base_Grid_Y = 0.0;
	_Base_Grid_Z = 0.0;

	_current_x = 0.0;
	_current_y = 0.0;
	_current_z = 0.0;

	if (NULL != _outfp)
	{
		fclose(_outfp);
		_outfp = NULL;
	}

	gridPool.clear();
	curvePool.clear();
	biCurveSurfacePool.clear();
	gridVector.clear();
	curveVector.clear();
	dul_curve_surfaceVector.clear();

}

void YPCLWriter::close(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	if (NULL == obj)
	{
		return;
	}

	// close file
	if (NULL != obj->_outfp)
	{
		fclose(obj->_outfp);
		obj->_outfp = NULL;
	}
	return;
}

void YPCLWriter::open(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	v8::String::Utf8Value str(args[0]->ToString());
	obj->_inputFilePath = *str;

	if (NULL == obj->_outputFilePath)
	{
		printf("PCLWriter::open: output file name is EMPTY.\n");
		return;
	}

	if (NULL == obj->_outfp)
	{
		printf("Please set output file path first.\n");
		return;
	}

	//open data file
	FILE *fp = fopen(obj->_inputFilePath, "r");
	// checking
	if (NULL == fp)
	{
		printf("Can't open %s\n", obj->_inputFilePath);
	}
	else
	{
		printf("Open %s success.\n", obj->_inputFilePath);
	}

	obj->sesHeader(obj->_outfp, "./", "section");

	int index = 0;

	// getting a line from the file
	int max = 1024;

	char *buffer = new char[2048];

	while (obj->getline(buffer, max, fp) != 0)
	{
		////		if( index >  40)
		////			break;

		// printf("line: %s\n", buffer);

		std::vector < std::string > dataList = obj->parserLine(buffer);

		std::vector<std::string>::iterator k = dataList.begin();

		std::string name = *k;
		dataList.erase(k);			//删除第一个元素

		std::string pclCommand = obj->_sectionPCLComandMap[name];

		obj->writePCLCommand(obj->_outfp, pclCommand, dataList);
	}

	delete[] buffer;
	// close file
	fclose(fp);
	return;
}
void YPCLWriter::setOutputFilePath(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	v8::String::Utf8Value str(args[0]->ToString());
	obj->_outputFilePath = *str;

	if (0 != obj->_outfp)
	{
		fclose(obj->_outfp);
		obj->_outfp = NULL;
		obj->_outputFilePath = NULL;
	}

	//open data file
	obj->_outfp = fopen(obj->_outputFilePath, "w");
	// checking
	if (NULL == obj->_outfp)
	{
		printf("Can't open %s\n", obj->_outputFilePath);
	}
	else
	{
		printf("Open %s success.\n", obj->_outputFilePath);
	}

	return;
}

void YPCLWriter::setPCLCommandFilePath(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	v8::String::Utf8Value str(args[0]->ToString());
	obj->_pclCommandFilePath = *str;

	//open bdf file
	FILE *fp = fopen(obj->_pclCommandFilePath, "r");
	// checking
	if (NULL == fp)
	{
		printf("Can't open %s\n", obj->_pclCommandFilePath);
	}
	else
	{
		printf("Open %s success.\n", obj->_pclCommandFilePath);
	}

	int index = 0;

	// getting a line from the file
	int max = 1024;

	//char buffer[2048];

	char * buffer = new char[2048];

	while (obj->getline(buffer, max, fp) != 0)
	{
		obj->parserPCLCommandLine(buffer);
		index++;
	}

	delete[] buffer;
	fclose(fp);

	return;
}
void YPCLWriter::setSectionPCLCommandMap(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 2)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	char *sectionType;
	v8::String::Utf8Value str(args[0]->ToString());
	sectionType = *str;

	char *pclCommand;
	v8::String::Utf8Value pclstr(args[1]->ToString());
	pclCommand = *pclstr;

	obj->_sectionPCLComandMap[sectionType] = pclCommand;

	return;
}

void YPCLWriter::write(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	v8::String::Utf8Value str(args[0]->ToString());
	// obj->parserSVGLine( *str );

	if (NULL == obj->_outfp)
	{
		printf("Please set output file path first.\n");
		return;
	}

	fprintf(obj->_outfp, *str);
	return;
}

void YPCLWriter::writePCL(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	int length = args.Length();
	if (2 == length)
	{

		v8::String::Utf8Value str(args[0]->ToString());
		char *pclCommandName = *str;

		if (args[1]->IsArray())
		{

			std::vector < std::string > dataList;
			Local<Array> ary = args[1].As<Array>();

			int length = ary->Length();
			for (int i = 0; i < length; i++)
			{
				Local<Value> value = ary->Get(i);

				v8::String::Utf8Value tmpStr(value->ToString());
				char *parameter = *tmpStr;

				dataList.push_back(parameter);
			}
			obj->writePCLCommand(obj->_outfp, pclCommandName, dataList);
		}

	}

	return;
}

void YPCLWriter::setBaseCenter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	int length = args.Length();
	if (1 == length)
	{
		if (args[0]->IsArray())
			;
		{
			Local<Array> ary = args[0].As<Array>();

			int length = ary->Length();
			if (3 != length)
			{
				printf(
						"setBaseCenter: Base center coordinate should has three elements. \n");
				return;
			}

			Local<Value> value = ary->Get(0);
			obj->_Base_Grid_X = double(value->NumberValue());
			value = ary->Get(1);
			obj->_Base_Grid_Y = double(value->NumberValue());
			value = ary->Get(2);
			obj->_Base_Grid_Z = double(value->NumberValue());

			// printf( "x = %f y = %f z = %f \n", obj->_Base_Grid_X, obj->_Base_Grid_Y, obj->_Base_Grid_Z);
		}
		return;
	}
	return;
}

void YPCLWriter::getBaseCenter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	v8::Handle<v8::Array> resultArray = v8::Array::New(isolate);
	resultArray->Set(v8::Integer::New(isolate, 0),
			v8::Number::New(isolate, obj->_Base_Grid_X));
	resultArray->Set(v8::Integer::New(isolate, 1),
			v8::Number::New(isolate, obj->_Base_Grid_Y));
	resultArray->Set(v8::Integer::New(isolate, 2),
			v8::Number::New(isolate, obj->_Base_Grid_Z));

	args.GetReturnValue().Set(resultArray);
	return;
}

void YPCLWriter::setCoordID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}


	int id = args[0]->Int32Value();
	obj->_coordID = id;

	return;
}

void YPCLWriter::getCoordID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	args.GetReturnValue().Set(v8::Int32::New(isolate, obj->_coordID));
	return;
}

void YPCLWriter::getCoordPlaneID(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	args.GetReturnValue().Set(v8::Int32::New(isolate, obj->_coordPlaneID));
	return;
}

void YPCLWriter::setCoordPlaneID(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());
	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}
	int id = args[0]->Int32Value();
	obj->_coordPlaneID = id;

	return;
}

void YPCLWriter::getGridID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	args.GetReturnValue().Set(v8::Int32::New(isolate, obj->GRID_ID));

	return;
}

void YPCLWriter::setGridID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());
	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}
	int id = args[0]->Int32Value();
	obj->GRID_ID = id;

	return;
}

void YPCLWriter::getCurveID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	args.GetReturnValue().Set(v8::Int32::New(isolate, obj->LINE_ID));

	return;
}

void YPCLWriter::setCurveID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());
	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}
	int id = args[0]->Int32Value();
	obj->LINE_ID = id;

	return;
}

void YPCLWriter::getBiCurveSurfaceID(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	args.GetReturnValue().Set(
			v8::Int32::New(isolate, obj->DUAL_CURVE_SURFACE_ID));

	return;
}
void YPCLWriter::setBiCurveSurfaceID(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());
	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}
	int id = args[0]->Int32Value();
	obj->DUAL_CURVE_SURFACE_ID = id;

	return;
}

void YPCLWriter::getStartID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	args.GetReturnValue().Set(v8::Int32::New(isolate, obj->_StartID));

	return;
}

void YPCLWriter::setStartID(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());
	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}
	int id = args[0]->Int32Value();
	obj->_StartID = id;

	return;
}

void YPCLWriter::writeSesFileHeader(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	obj->sesHeader(obj->_outfp, "./", "section");

	return;
}

//----------------------------------------------------------
void YPCLWriter::parserPCLCommandLine(char *buffer)
{
	// checking the input buffer
	if (NULL == buffer)
	{
		return;
	}

	std::vector < std::string > dataList;

	char delims[] = "\t";
	char *tmp = NULL;
	tmp = strtok(buffer, delims);

	while (tmp != NULL)
	{

		//printf( "para is \"%s\"\n", tmp );

		//theCard.pushEntry(  tmp, _cardFiledSize);
		dataList.push_back(tmp);

		tmp = strtok(NULL, delims);
	}

	std::vector<std::string>::iterator k = dataList.begin();

	std::string name = *k;

	//  printf ("PCL name: %s\n",name.c_str());;

	dataList.erase(k);    //删除第一个元素

	_pclComandMap[name] = dataList;

	return;
}

//----------------------------------------------------------
std::vector<std::string> YPCLWriter::parserLine(char *buffer)
{
	std::vector < std::string > dataList;

	// checking the input buffer
	if (NULL == buffer)
	{
		return dataList;
	}

	char delims[] = "\t";
	char *tmp = NULL;
	tmp = strtok(buffer, delims);

	while (tmp != NULL)
	{

		// printf( "parserLine:  tmp = %s \n", tmp );

		int fieldSize = (unsigned) strlen(tmp);

		char* tmpStr = (char*) malloc((1 + fieldSize) * sizeof(char));

		if (NULL == tmpStr)
		{
			printf("allocating memory failed. \n");
			return dataList;
		}

		int trimedLength = trimwhitespace(tmpStr, fieldSize + 1, tmp);

		dataList.push_back(tmpStr);

		tmp = strtok(NULL, delims);
	}
	return dataList;
}

void YPCLWriter::sesHeader(FILE *outfp, char* theCurrentDir, char* theJobName)
{

	if (NULL == outfp)
	{
		printf("Please set output file path first.\n");
		return;
	}

	fprintf(outfp, "$#   \n");
	fprintf(outfp, "$#    Created by PCLWriter\n");
	fprintf(outfp, "$#   \n");
	fprintf(outfp, "$#    Auther: Chunfeng Yang\n");
	fprintf(outfp, "$#    Email: yangchunfeng@mail.dlut.edu.cn\n");
	fprintf(outfp, "$#    Version: %s \n", _version);
	fprintf(outfp, "$#   \n");

//	fprintf(outfp, "  \r\n");
//	fprintf(outfp, "  \r\n");
//
//	fprintf(outfp, "$# \r\n");
//	fprintf(outfp, "$#  Global variables  \r\n");
//	fprintf(outfp, "$#  \r\n");
//
	fprintf(outfp, "STRING s_output_ids[VIRTUAL]\r\n");
//	fprintf(outfp, "INTEGER i_ncurves\r\n");
	fprintf(outfp, "INTEGER i_return_value\r\n");
//	fprintf(outfp, "REAL r_return_value\r\n");
//	fprintf(outfp, "STRING s_plane_list[VIRTUAL]\r\n");
//
//	fprintf(outfp, "INTEGER i_temp_value\r\n");
//	fprintf(outfp, "REAL r_temp_value\r\n");
//	fprintf(outfp, "STRING s_temp_value[VIRTUAL]\r\n");
//
//	fprintf(outfp, "INTEGER i_total_node_number\r\n");
//
//	fprintf(outfp, "INTEGER fem_create_mesh_curve_num_nodes\r\n");
//	fprintf(outfp, "INTEGER fem_create_mesh_curve_num_elems\r\n");
//
//	fprintf(outfp, "STRING select_element_var_name[25]\r\n");
//	fprintf(outfp, "STRING s_point_list[6000]\r\n");
//	fprintf(outfp, "STRING s_point_1_list[100]\r\n");
//	fprintf(outfp, "STRING s_point_2_list[100]\r\n");
//	fprintf(outfp, "STRING s_point_3_list[100]\r\n");
//
//	fprintf(outfp, "INTEGER i_order\r\n");
//	fprintf(outfp, "LOGICAL l_interpolate\r\n");
//	fprintf(outfp, "INTEGER i_param_method\r\n");
//	fprintf(outfp, "LOGICAL l_closed\r\n");
//	fprintf(outfp, "STRING sv_created_ids[VIRTUAL]\r\n");

	fprintf(outfp, "$#  \r\n");
	fprintf(outfp, "$#  \r\n");

	fprintf(outfp, "set_current_dir( \"%s\" ) \r\n", theCurrentDir);
	fprintf(outfp, "uil_file_new.go(\"\", \"%s.db\" ) \r\n", theJobName);
	fprintf(outfp, "$? YES 36000002  \r\n");

	fprintf(outfp, "i_return_value = ga_view_aa_set( 23, -34, 0) \r\n");
	fprintf(outfp, "dump i_return_value   \r\n");
}

void YPCLWriter::writePCLCommand(FILE *outfp, std::string name,
		const std::vector<std::string> &vec)
{
	if (NULL == outfp)
	{
		printf("Please set output file path first.\n");
		return;
	}

	//  DEBUG
	// printf( " YPCLWriter::writePCLCommand:   \n" );
	// printf( "command   %s  ", name.c_str() );
	//  DEBUG

	std::vector < std::string > parameters = _pclComandMap[name];

	int paraVectorLength = parameters.size();
	int paraDataNumber = paraVectorLength / 3;

	//  DEBUG
	// printf( "parameters   %s  ", name );
	// printf( "parameters Number = %d  ", paraDataNumber );
	//

	if (0 == parameters.size())
	{
		return;
	}

	int length = vec.size();
	// DEBUG
	//printf( "data Number = %d  \n", length );

//	   std::stringstream ss;
//	   for(size_t i = 0; i < length; ++i)
//	   {
//	     if(i != 0)
//	     {
//	       ss << ",";
//	     }
//	     ss << v[i];
//	   }
//	   std::string s = ss.str();

	int sectionLength = name.size();

	fprintf(outfp, "%s(", name.c_str());

	for (int i = 0; i < length; i++)
	{

		if (i > paraDataNumber - 1)
		{
			break;
		}

		std::string paraData = vec[i];
		std::string paraType = parameters[i * 3 + 1];

		// printf( "\n writePCLCommand: paraName = %s  paraType = %s \n", parameters[i * 3].c_str(),   paraType.c_str() );
		// printf( "writePCLCommand:  paraData = %s \n", paraData.c_str() );

		// fprintf(outfp, "s_output_ids = \"#\"   \r\n");

		if (i != 0)
		{
			fprintf(outfp, ",");
		}
		if (sectionLength > 50)
		{
			fprintf(outfp, "@\n");
			sectionLength = 0;
		}

		std::size_t found;

		found = paraType.find("STRING");
		if (found != std::string::npos)
		{
			fprintf(outfp, " %s", paraData.c_str());
		}

		found = paraType.find("INTEGER");
		if (found != std::string::npos)
		{
			fprintf(outfp, " %s", paraData.c_str());

		}

		found = paraType.find("REAL");
		if (found != std::string::npos)
		{
			fprintf(outfp, " %s", paraData.c_str());

		}

		found = paraType.find("LOGICAL");
		if (found != std::string::npos)
		{
			fprintf(outfp, " %s", paraData.c_str());
		}

//		found = paraType.find("LIST");
//		if (found != std::string::npos)
//		{
//			fprintf(outfp, " %s", paraData.c_str());
//		}

		sectionLength += paraData.size();
	}

	fprintf(outfp, " ) \n");

	return;
}

char* YPCLWriter::fgets(char *s, int n, FILE *iop)
{
	int c;
	char *cs;

	cs = s;
	while (--n > 0 && (c = getc(iop)) != EOF)
	{
		if ((*cs++ = c) == '\n')
		{
			break;
		}
	}
	*cs = '\0';
	return (c == EOF && cs == s) ? NULL : s;
}

int YPCLWriter::fputs(char *s, FILE *iop)
{
	int c;

	while (c = *s++)
	{
		putc(c, iop);
	}
	return ferror(iop) ? EOF : 0;
}

//
// Read a line from the file
int YPCLWriter::getline(char *line, int max, FILE *fp)
{
	if (fgets(line, max, fp) == NULL)
	{
		return 0;
	}
	else
	{
		return strlen(line);
	}

}

// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is
// truncated.
size_t YPCLWriter::trimwhitespace(char *out, size_t len, const char *str)
{
	if (len == 0)
		return 0;

	const char *end;
	size_t out_size;

	// Trim leading space
	while (isspace(*str))
		str++;

	if (*str == 0)  // All spaces?
	{
		*out = 0;
		return 1;
	}

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		end--;
	end++;

	// Set output size to minimum of trimmed string length and buffer size minus 1
	out_size = (end - str) < len - 1 ? (end - str) : len - 1;

	// Copy trimmed string and add null terminator
	memcpy(out, str, out_size);
	out[out_size] = 0;

	return out_size;
}

//
//  svg path parser
//

Grid YPCLWriter::getGrid(int id)
{
	Grid theGrid;
	int length = gridPool.size();
	for (int i = 0; i < length; i++)
	{
		theGrid = gridPool[i];
		if (id == theGrid._ID)
		{
			return theGrid;
		}
	}
	theGrid._ID = -1;
	return theGrid;
}

Curve YPCLWriter::getCurve(int id)
{
	Curve theCurve;
	int length = curvePool.size();
	for (int i = 0; i < length; i++)
	{
		Curve aCurve = curvePool[i];
		if (id == aCurve._ID)
		{
			return aCurve;
		}
	}
	return theCurve;
}

int YPCLWriter::saveArc(int startGridID, int endGridID, double radius,
		double rotation, int clockwise, int large_arc_angle)
{
	Grid startGrid = getGrid(startGridID);
	Grid endGrid = getGrid(endGridID);

	// dataList.push_back( " " );
//
//	arcMethod: 1 -- define arc by center, start and end points.
//	           2 -- define arc by start point, end point and radius.
//
	int arcMethod = 2; //

	LINE_ID++;

	std::vector < std::string > dataList;
	dataList.push_back(SSTR("\"" << LINE_ID << "\""));

	//dataList.push_back( "2 " );
	dataList.push_back(SSTR(arcMethod));
	dataList.push_back(SSTR(radius));
	dataList.push_back("false ");  // true   -- create center

//
	if (1 == clockwise)
	{
		dataList.push_back("true ");
	}
	else
	{
		dataList.push_back("false ");
	}

	if (1 == large_arc_angle)
	{
		dataList.push_back("0 ");
	}
	if (0 == large_arc_angle)
	{
		dataList.push_back("1 ");
	}

	dataList.push_back(
			SSTR("\"Coord " << _coordID << "." << _coordPlaneID << "\""));
	dataList.push_back(" \"\" ");
	dataList.push_back(SSTR("\"Point " << startGridID << "\""));
	dataList.push_back(SSTR("\"Point " << endGridID << "\""));
	dataList.push_back("false ");
	dataList.push_back("s_output_ids ");

	writePCLCommand(_outfp, "sgm_const_curve_2d_arc2point_v2", dataList);

	return LINE_ID;
}

int YPCLWriter::saveGrid(double x, double y, double z)
{
	Grid theGrid;
	double error = 0.001;

	theGrid._X = x;
	theGrid._Y = y;
	theGrid._Z = z;
	theGrid._CoordID = _coordID;
	int length = gridPool.size();

	theGrid._X += _Base_Grid_X;
	theGrid._Y += _Base_Grid_Y;
	theGrid._Z += _Base_Grid_Z;

//
//	printf( "saveGrid(double x, double y, double z) \n" );
//	printf( "theGrid._X = %f \n", theGrid._X );
//	printf( "theGrid._Y = %f \n", theGrid._Y );
//	printf( "theGrid._Z = %f \n", theGrid._Z );
//
//

	for (int i = 0; i < length; i++)
	{
		Grid aGrid = gridPool[i];
		if (theGrid.isSame(aGrid, error))
		{
			printf("Duplicate grid. \n");
			return aGrid._ID;
		}
	}

	GRID_ID++;
	theGrid._ID = GRID_ID;
	gridPool.push_back(theGrid);

//	std::cout << "Grid\t" << "\"" << theGrid._ID << "\"\t";
//	std::cout << "\"[" << theGrid._X << ", " << theGrid._Y << ", " << theGrid._Z
//			<< "]\"\t";
//	std::cout << "\"Coord 0\"\t s_output_ids";
//	std::cout << std::endl;

	std::vector < std::string > dataList;
	dataList.push_back(SSTR("\"" << theGrid._ID << "\""));
	dataList.push_back(
			SSTR(
					"\"[" << theGrid._X << ", " << theGrid._Y << ", " << theGrid._Z << "]\" "));

	dataList.push_back(SSTR("\"Coord " << _coordID << "\""));
	dataList.push_back("s_output_ids ");

	writePCLCommand(_outfp, "asm_const_grid_xyz", dataList);

	return GRID_ID;
}

int YPCLWriter::saveGrid(double coord_1, double coord_2)
{
	Grid theGrid;
	double error = 0.001;

	theGrid._X = 0;
	theGrid._Y = 0;
	theGrid._Z = 0;
	theGrid._CoordID = _coordID;
	int length = gridPool.size();

	switch (_coordPlaneID)
	{
	case 1:   // YZ plane
		//var transYZ = [ [0, 1, 0], [0, 0, 1] ];
		//var tmp = numeric.dot (  myGridCoord, transYZ ) ;
		theGrid._Y = coord_1;
		theGrid._Z = coord_2;
		break;
	case 2:   // ZX plane
		//var transXZ = [ [1, 0, 0], [0, 0, 1] ];
		//var tmp = numeric.dot (  myGridCoord, transXZ ) ;

		theGrid._Z = coord_1;
		theGrid._X = coord_2;
		break;
	case 3:   // XY plane
		//var transXY = [ [1, 0, 0], [0, 1, 0] ];
		//var tmp = numeric.dot (  myGridCoord, transXY ) ;
		theGrid._X = coord_1;
		theGrid._Y = coord_2;
		break;
	default:
		printf(
				"saveGrid: _coordPlaneID = %d is incorrect. Please reset coordPlaneID.",
				_coordPlaneID);
		return -1;
	}

	theGrid._X += _Base_Grid_X;
	theGrid._Y += _Base_Grid_Y;
	theGrid._Z += _Base_Grid_Z;

	for (int i = 0; i < length; i++)
	{
		Grid aGrid = gridPool[i];
		if (theGrid.isSame(aGrid, error))
		{
			printf("Duplicate grid. \n");
			return aGrid._ID;
		}
	}

	GRID_ID++;
	theGrid._ID = GRID_ID;
	gridPool.push_back(theGrid);

//	std::cout << "Grid\t" << "\"" << theGrid._ID << "\"\t";
//	std::cout << "\"[" << theGrid._X << ", " << theGrid._Y << ", " << theGrid._Z
//			<< "]\"\t";
//	std::cout << "\"Coord 0\"\t s_output_ids";
//	std::cout << std::endl;

	std::vector < std::string > dataList;
	dataList.push_back(SSTR("\"" << theGrid._ID << "\""));
	dataList.push_back(
			SSTR(
					"\"[" << theGrid._X << ", " << theGrid._Y << ", " << theGrid._Z << "]\" "));

	dataList.push_back(SSTR("\"Coord " << _coordID << "\""));
	dataList.push_back("s_output_ids ");

	writePCLCommand(_outfp, "asm_const_grid_xyz", dataList);

	return GRID_ID;
}

void YPCLWriter::createGrid(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 3)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	double x = args[0]->IsUndefined() ? 0.0 : args[0]->NumberValue();
	double y = args[1]->IsUndefined() ? 0.0 : args[1]->NumberValue();
	double z = args[2]->IsUndefined() ? 0.0 : args[2]->NumberValue();

	int grid_ID = obj->saveGrid(x, y, z);

	args.GetReturnValue().Set(v8::Int32::New(isolate, grid_ID));
	return;
}

void YPCLWriter::getGrid(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	int grid_ID = args[0]->Int32Value();

	Grid theGrid = obj->getGrid(grid_ID);

	v8::Handle<v8::Array> gridArray = v8::Array::New(isolate);

	gridArray->Set(v8::Integer::New(isolate, 0),
			v8::Integer::New(isolate, theGrid._ID));
	gridArray->Set(v8::Integer::New(isolate, 1),
			v8::Number::New(isolate, theGrid._X));
	gridArray->Set(v8::Integer::New(isolate, 2),
			v8::Number::New(isolate, theGrid._Y));
	gridArray->Set(v8::Integer::New(isolate, 3),
			v8::Number::New(isolate, theGrid._Z));
	gridArray->Set(v8::Integer::New(isolate, 4),
			v8::Integer::New(isolate, theGrid._CoordID));

	args.GetReturnValue().Set(gridArray);

	return;
}

void YPCLWriter::getCurve(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	int theID = args[0]->Int32Value();

	Curve theCurve = obj->getCurve(theID);

	v8::Handle<v8::Array> curveArray = v8::Array::New(isolate);

	curveArray->Set(v8::Integer::New(isolate, 0),
			v8::Integer::New(isolate, theCurve._ID));
	curveArray->Set(v8::Integer::New(isolate, 1),
			v8::Number::New(isolate, theCurve._startGridID));
	curveArray->Set(v8::Integer::New(isolate, 2),
			v8::Number::New(isolate, theCurve._endGridID));

	args.GetReturnValue().Set(curveArray);
	return;
}

void YPCLWriter::createCurve(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 2)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	int grid_1_ID = args[0]->Int32Value();
	int grid_2_ID = args[1]->Int32Value();

	int curve_ID = obj->saveLine(grid_1_ID, grid_2_ID);

	args.GetReturnValue().Set(v8::Int32::New(isolate, curve_ID));
	return;
}

void YPCLWriter::constructBiCurveSurface(
		const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 2)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	int curve_1_ID = args[0]->Int32Value();
	int curve_2_ID = args[1]->Int32Value();

	int surface_ID = obj->saveBiCurveSurface(curve_1_ID, curve_2_ID);

	args.GetReturnValue().Set(v8::Int32::New(isolate, surface_ID));
	return;
}

int YPCLWriter::saveBiCurveSurface(int startID, int endID)
{
	Curve theSurface;

	theSurface._startGridID = startID;
	theSurface._endGridID = endID;

	int length = biCurveSurfacePool.size();

	for (int i = 0; i < length; i++)
	{
		Curve aSurface = biCurveSurfacePool[i];
		if (theSurface.isSame(aSurface))
		{
			printf("Duplicate 2_curvesurface. \n");
			return aSurface._ID;
		}
	}

	DUAL_CURVE_SURFACE_ID++;
	theSurface._ID = DUAL_CURVE_SURFACE_ID;

	biCurveSurfacePool.push_back(theSurface);

	std::vector < std::string > dataList;

	dataList.push_back(SSTR("\"" << theSurface._ID << "\""));
	dataList.push_back(SSTR("\"Curve " << startID << "\""));
	dataList.push_back(SSTR("\"Curve " << endID << "\""));
	dataList.push_back("s_output_ids");

	int paraDataNumber = dataList.size();
	// printf( "YPCLWriter::saveLine:  parameters Number = %d  \n", paraDataNumber );

	std::string name = "asm_const_surface_2curve";

	writePCLCommand(_outfp, name, dataList);

	return DUAL_CURVE_SURFACE_ID;
}

int YPCLWriter::saveLine(int startGridID, int endGridID)
{
	//
	// Checking input parameters
	//
	if ((startGridID < 0) || (endGridID < 0))
	{
		printf("Invalid grid ID in save line. \n");
		return -1;
	}
	if (startGridID == endGridID < 0)
	{
		printf(
				"Invalid grid ID in save line: startGridID %d == endGridID %d. \n",
				startGridID, endGridID);
		return -1;
	}

	Curve theCurve;

	theCurve._startGridID = startGridID;
	theCurve._endGridID = endGridID;

	int length = curvePool.size();

	for (int i = 0; i < length; i++)
	{
		Curve aCurve = curvePool[i];
		if (theCurve.isSame(aCurve))
		{
			printf("Duplicate curve. \n");
			return aCurve._ID;
		}
	}

	LINE_ID++;
	theCurve._ID = LINE_ID;
	curvePool.push_back(theCurve);

	std::vector < std::string > dataList;

	dataList.push_back(SSTR("\"" << theCurve._ID << "\""));

	dataList.push_back(SSTR("\"Point " << startGridID << "\""));
	dataList.push_back(SSTR("\"Point " << endGridID << "\""));
	dataList.push_back("0 ");
	dataList.push_back(" \"\" ");

	dataList.push_back("50 ");
	dataList.push_back("1 ");
	dataList.push_back("s_output_ids");

	int paraDataNumber = dataList.size();
	// printf( "YPCLWriter::saveLine:  parameters Number = %d  \n", paraDataNumber );

	std::string name = "asm_const_line_2point";

	writePCLCommand(_outfp, name, dataList);

//	std::cout << "Line\t" << "\"" << theCurve._ID << "\"\t";
//	std::cout << "\"Point " << startGridID << "\"\t" << "\"Point "
//			<< endGridID << "\"\t 0\t";
//	std::cout << "\"\" \t 50\t 1\t s_output_ids";
//	std::cout << std::endl;

	return LINE_ID;
}

void YPCLWriter::parserSVGLine(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	YPCLWriter* obj = ObjectWrap::Unwrap<YPCLWriter>(args.Holder());

	// checking arguments number
	if (args.Length() < 1)
	{
		isolate->ThrowException(
				v8::Exception::TypeError(
						String::NewFromUtf8(isolate,
								"Wrong arguments number")));
		return;
	}

	v8::String::Utf8Value str(args[0]->ToString());

	obj->parserSVGLine(*str);

//	  for( int i = 0; i < length; i++ )
//	  {
//		  int id = obj->curveVector[i];
//	      resultArray->Set(v8::Integer::New(isolate, i), v8::Integer::New(isolate, id ));
//	  }
//      args.GetReturnValue().Set(resultArray);

	//  Results
	//
	v8::Handle<v8::Array> resultArray = v8::Array::New(isolate);

	v8::Handle<v8::Array> gridArray = v8::Array::New(isolate);
	v8::Handle<v8::Array> curveArray = v8::Array::New(isolate);

	int length = obj->gridVector.size();
	for (int i = 0; i < length; i++)
	{
		int id = obj->gridVector[i];
		gridArray->Set(v8::Integer::New(isolate, i),
				v8::Integer::New(isolate, id));
	}

	length = obj->curveVector.size();
	for (int i = 0; i < length; i++)
	{
		int id = obj->curveVector[i];
		curveArray->Set(v8::Integer::New(isolate, i),
				v8::Integer::New(isolate, id));
	}

	resultArray->Set(v8::Integer::New(isolate, 0), gridArray);
	resultArray->Set(v8::Integer::New(isolate, 1), curveArray);

	args.GetReturnValue().Set(resultArray);

	return;
}

//----------------------------------------------------------
void YPCLWriter::parserSVGLine(char *buffer)
{

	_current_z = 0.0;

	gridVector.clear();
	curveVector.clear();

	std::vector < std::string > dataList;

	int startID = 0;

	// checking the input buffer
	if (NULL == buffer)
	{
		// return dataList;
		return;
	}

	//
	// std::cout << "Buffer: " << buffer << std::endl;
	// printf( "%s \n", buffer );
	//

	char delims[] = " \t";
	char *tmp = NULL;
	tmp = strtok(buffer, delims);

	while (tmp != NULL)
	{
		// printf( "parserLine:  tmp = %s \n", tmp );

		int fieldSize = (unsigned) strlen(tmp);

		char* tmpStr = (char*) malloc((1 + fieldSize) * sizeof(char));

		if (NULL == tmpStr)
		{
			printf("allocating memory failed. \n");
			return; // dataList;
		}

		int trimedLength = trimwhitespace(tmpStr, fieldSize + 1, tmp);

		//
		// std::cout << tmpStr << std::endl;
		//

		//std::cout << tmp << std::endl;

		dataList.push_back(tmpStr);

		tmp = strtok(NULL, delims);
	}
	// return dataList;

	int length = dataList.size();

	for (int i = 0; i < length; i++)
	{
		//char *buffer = dataList[i].c_str();

		char *cstr = new char[dataList[i].length() + 1];
		strcpy(cstr, dataList[i].c_str());

		tmp = strstr(cstr, "M");
		if (tmp)
		{
			char* p;

			i++;
			if (i >= length)
			{
				return;
			}
			double x = strtod(dataList[i].c_str(), &p);
			if (*p)
			{
				printf(
						"Grid coordination conversion failed because the input wasn't a number. \n");
			}

//          // old version
//			double x = (double) atof(dataList[i].c_str() );

			i++;
			if (i >= length)
			{
				return;
			}
			double y = strtod(dataList[i].c_str(), &p);
			if (*p)
			{
				printf(
						"Grid coordination conversion failed because the input wasn't a number. \n");
			}

			i++;
			if (i >= length)
			{
				i--;
				_current_x = x;
				_current_y = y;

				startID = saveGrid(x, y);
				gridVector.push_back(startID);

				//
				//printf( "M  - saveGrid( %f, %f ) -> %i \n", x, y, startID );

				return;
			}
			double z = strtod(dataList[i].c_str(), &p);
			if (*p)
			{
				i--;
				_current_x = x;
				_current_y = y;

				startID = saveGrid(x, y);
				gridVector.push_back(startID);

				//
				//printf( "M  - saveGrid( %f, %f ) -> %i \n", x, y, startID );
				//

			}
			else
			{
				_current_x = x;
				_current_y = y;
				_current_z = z;

				//
				//printf( "M  - saveGrid( %f, %f, %f )  -> %i  \n", x, y, z, startID );
				//

				startID = saveGrid(x, y, z);
				gridVector.push_back(startID);
			}

		}

		tmp = strstr(cstr, "d");
		if (tmp)
		{
			i++;
			if (i >= length)
			{
				return;
			}
			double dx = (double) atof(dataList[i].c_str());
			i++;
			if (i >= length)
			{
				return;
			}
			double dy = (double) atof(dataList[i].c_str());

			_current_x += dx;
			_current_y += dy;

			startID = saveGrid(_current_x, _current_y);
			gridVector.push_back(startID);
		}

		tmp = strstr(cstr, "L");
		if (tmp)
		{
			char* p;

			i++;
			if (i >= length)
			{
				return;
			}
			double x = strtod(dataList[i].c_str(), &p);
			if (*p)
			{
				printf(
						"Grid coordination conversion failed because the input wasn't a number. \n");
			}

			i++;
			if (i >= length)
			{
				return;
			}
			double y = strtod(dataList[i].c_str(), &p);
			if (*p)
			{
				printf(
						"Grid coordination conversion failed because the input wasn't a number. \n");
			}

			i++;
			if (i >= length)
			{
				i--;

				_current_x = x;
				_current_y = y;

				int gridID = saveGrid(x, y);

				int lineID = saveLine(startID, gridID);

				gridVector.push_back(gridID);
				curveVector.push_back(lineID);

				startID = gridID;

				return;
			}
			double z = strtod(dataList[i].c_str(), &p);
			if (*p)
			{
				i--;

				_current_x = x;
				_current_y = y;

				int gridID = saveGrid(x, y);

				int lineID = saveLine(startID, gridID);

				gridVector.push_back(gridID);
				curveVector.push_back(lineID);

				startID = gridID;
			}
			else
			{
				_current_x = x;
				_current_y = y;
				_current_z = z;

				//
				// printf( "L  - saveGrid( %f, %f, %f )  -> %i  \n", x, y, z, startID );
				//

				int gridID = saveGrid(x, y, z);

				int lineID = saveLine(startID, gridID);

				gridVector.push_back(gridID);
				curveVector.push_back(lineID);

				startID = gridID;
			}

//			_current_x = x;
//			_current_y = y;
//			//
//			//std::cout << x << " " << y << " " << " " << section_A_Z
//			//		<< std::endl;
//			// printf( "grid:[%f %f %f] \n", x, y, section_A_Z );
//			//
//
//			int gridID = saveGrid(x, y );
//
//			int lineID = saveLine( startID, gridID );
//
//			gridVector.push_back( gridID );
//			curveVector.push_back( lineID );
//
//			startID = gridID;
		}

		tmp = strstr(cstr, "l");
		if (tmp)
		{
			i++;
			if (i >= length)
			{
				return;
			}
			double dx = (double) atof(dataList[i].c_str());
			i++;
			if (i >= length)
			{
				return;
			}
			double dy = (double) atof(dataList[i].c_str());

			_current_x += dx;
			_current_y += dy;
			int gridID = saveGrid(_current_x, _current_y);
			int lineID = saveLine(startID, gridID);

			gridVector.push_back(gridID);
			curveVector.push_back(lineID);

			startID = gridID;
		}

		tmp = strstr(cstr, "A");
		if (tmp)
		{

			i++;
			if (i >= length)
			{
				return;
			}
			double Rx = (double) atof(dataList[i].c_str());
			i++;
			if (i >= length)
			{
				return;
			}
			double Ry = (double) atof(dataList[i].c_str());

			i++;
			if (i >= length)
			{
				return;
			}
			double rotation = (double) atof(dataList[i].c_str());
			i++;
			if (i >= length)
			{
				return;
			}
			int arc_angle = (int) atoi(dataList[i].c_str());

			i++;
			if (i >= length)
			{
				return;
			}
			int clockwise = (int) atoi(dataList[i].c_str());

			i++;
			if (i >= length)
			{
				return;
			}
			double x = (double) atof(dataList[i].c_str());
			i++;
			if (i >= length)
			{
				return;
			}
			double y = (double) atof(dataList[i].c_str());
			//
			//std::cout << x << " " << y << " " << " " << section_A_Z
			//		<< std::endl;
			//

			_current_x = x;
			_current_y = y;

			int gridID = saveGrid(_current_x, _current_y);

			int lineID = saveArc(startID, gridID, Rx, rotation, clockwise,
					arc_angle);

			gridVector.push_back(gridID);
			curveVector.push_back(lineID);

			startID = gridID;
		}
	}
}

