# pclwriter
Generating pcl file for Patran in Node.js (v 4.3.0)

# Examples 
<pre>
//
//  Modules 
var PCLParser = require( '../build/Release/pclwriter' ); 
 
//  Global Variables
//
var theParser = new PCLParser.YPCLWriter();
var SVGLine = 'M 202.637 108.597 L 201.448 105.483 A 2.29999 2.29999 0 0 0 199.388 104.006';

theParser.setSectionPCLCommandMap("Loftcurve", "sgm_const_curve_loft" );
theParser.setSectionPCLCommandMap("Arc2", "sgm_const_curve_2d_arc2point_v2" );
theParser.setSectionPCLCommandMap("2CurSur", 'asm_const_surface_2curve');


theParser.setSectionPCLCommandMap("Grid", 'asm_const_grid_xyz');
theParser.setSectionPCLCommandMap("Line", 'asm_const_line_2point');

theParser.setPCLCommandFilePath('../msc/pclcom.txt');
theParser.setOutputFilePath('./section.ses');
  
theParser.write('STRING s_output_ids[VIRTUAL]\n');
theParser.setBaseCenter([10.1, 12.2, 10.0]);
theParser.setCoordPlaneID(2)

theParser.writeSesFileHeader();

var result = theParser.parserSVGLine( SVGLine );

console.log( result );

function test()
{
    var commandName = 'asm_const_grid_xyz'    
	var paraList = [];
	paraList.push( '\"#\"' );
	paraList.push( '\"[200, 100, 118 ]\"' );
	paraList.push( '\"Coord 0\"' );
	paraList.push( 's_output_ids' );

	theParser.writePCL( commandName, paraList ); 				
}

//
//  Loading and Constraints
function load()
{
    var command = 'loadsbcs_create2'    
	var paraList = [];
    
    var loadName = '\"Disp_1\"'
    var loadType = '\"Displacement\"';
 
	paraList.push( loadName );
	paraList.push( loadType );
	paraList.push( '\"Nodal\"' );
	paraList.push( '\" \"' );
	paraList.push( '\"Static\"' );
	paraList.push( '[\"Node 10001 10002\"]' );
	paraList.push( '\"FEM\"' );
	paraList.push( '\"Coord 0\"' );
	paraList.push( '\"1.0\"' );
	paraList.push( ' [\"<0 0 0>\", \"<0 0 0>\", \"< >\", \"< >\"]' );
	paraList.push( '[\"\", \"\", \"\", \"\"]' );

	theParser.writePCL( command, paraList );

    paraList = [];  

    loadName = '\"F_1\"';
    loadType = '\"Force\"';

	paraList.push( loadName );
	paraList.push( loadType );
	paraList.push( '\"Nodal\"' );
	paraList.push( '\" \"' );
	paraList.push( '\"Static\"' );
	paraList.push( '[\"Node 10003\"]' );
	paraList.push( '\"FEM\"' );
	paraList.push( '\"Coord 0\"' );
	paraList.push( '\"1.0\"' );
	paraList.push( ' [\"<0 0   0 >\", \"<1000 0 0>\", \"< >\", \"< >\"]' );
	paraList.push( '[\"\", \"\", \"\", \"\"]' );

	theParser.writePCL( command, paraList );
}

test();
load();
</pre>
