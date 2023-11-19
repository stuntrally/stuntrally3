#include "Grid.h"
#include "Def_Str.h"
// #include <Ogre.h>
using namespace Ogre;
using namespace std;


void GridCellMesh::Clear()
{
	pos.clear();  norm.clear();
	tcs.clear();  clr.clear();
	idx.clear();
}


//  grid cells map
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void GridCells::Clear()
{	
	cells.clear();
}

//  world pos to grid cell index
GridPos GridCells::From(Vector3 worldPos)
{
#if 0  // test no cells
	return GridPos(0,0,0);
#else
	return make_tuple(
		worldPos.x / fCellSize,
		worldPos.y / fCellSize,
		worldPos.z / fCellSize);
#endif
}


void GridCells::AddMesh(int lod,
	Vector3 worldPos,
	GridMtr mtr,
	const std::vector<Vector3>& pos, const std::vector<Vector3>& norm,
	const std::vector<Vector4>& clr, const std::vector<Vector2>& tcs,
	const std::vector<uint16>& idx)
{
	GridPos gpos = From(worldPos);
	// short x,y,z;  tie(x,y,z) = gpos;
	// LogO("G  x "+toStr(x)+" y "+toStr(y)+" z "+toStr(z));
	// cells.insert_or_assign(gpos, );

	String s;
	auto it = cells.find(gpos);
	if (it != cells.end())
	{
		s = "Grid: add  ";
		GridCellMesh& m = (*it).second.lods[lod];
		uint16 o = m.pos.size();  // max 64k!

		#define add(a,b)  a.insert(a.end(), b.begin(), b.end())
		// LogO("a " + toStr(m.pos.size())+" + " + toStr(pos.size()));
		add(m.pos, pos);
		add(m.norm,norm);
		add(m.clr, clr);  // all must have or none!
		add(m.tcs, tcs);

		//  ofset idx, by current size
		std::vector<uint16> ofs;
		ofs.reserve(idx.size());

		for (auto i : idx)
			ofs.push_back(i + o);

		add(m.idx, ofs);
		#undef add
	}else
	{
		s = "Grid: new  ";
		GridCellMesh m;
		m.pos = pos;  m.norm = norm;  m.clr = clr;
		m.tcs = tcs;  m.idx = idx;

		GridCellLods gcl;
		gcl.gpos = gpos;
		gcl.mtr = mtr;
		gcl.lods[lod] = m;
		cells[gpos] = gcl;  // new
	}
	// LogO(s + mtr.name+"  pos "+toStr(pos.size())+" idx "+toStr(idx.size())+"  wp "+toStr(worldPos));
}


void GridCells::Create()
{
	LogO("C:## Grid + cells: "+toStr(cells.size()));
	for (auto& c : cells)
		c.second.Create();
}

void GridCells::Destroy()
{
	for (auto& c : cells)
		c.second.Destroy();
	cells.clear();
}


//  whole grid, all materials
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void GridMtrs::Clear()
{	
	mtrs.clear();
}

void GridMtrs::AddMesh(
	int lod,
	Vector3 worldPos,
	GridMtr mtr,
	const std::vector<Vector3>& pos, const std::vector<Vector3>& norm,
	const std::vector<Vector4>& clr, const std::vector<Vector2>& tcs,
	const std::vector<uint16>& idx)
{
	String s;
	auto it = mtrs.find(mtr);
	if (it != mtrs.end())
	{
		s = "Grid add  ";
		(*it).second.AddMesh(lod, worldPos, mtr,
			pos, norm, clr, tcs, idx);
	}else
	{
		GridCells cells;
		// cells.fCellSize = 10.f;  // par-

		s = "Grid New  ";
		cells.AddMesh(lod, worldPos, mtr,
			pos, norm, clr, tcs, idx);
		mtrs[mtr] = cells;  // new
	}
	LogO(s + mtr.name +" lod "+toStr(lod)+"  pos "+toStr(pos.size())+
		"  clr "+toStr(clr.size())+" idx "+toStr(idx.size())+"  wp "+toStr(worldPos));
}


void GridMtrs::Create()
{
	LogO("C:## Grid + Create ##");
	LogO("C::+ Grid = materials: "+toStr(mtrs.size()));
	int meshes = 0;
	for (const auto& m : mtrs)
		meshes += m.second.cells.size();
	LogO("C::+ Grid = meshes: "+toStr(meshes));

	for (auto& m : mtrs)
		m.second.Create();
}

void GridMtrs::Destroy()
{
	LogO("D:## Grid - Destroy ##");
	for (auto& m : mtrs)
		m.second.Destroy();
	mtrs.clear();
}
