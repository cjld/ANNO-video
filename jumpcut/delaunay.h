/*
 *  delaunay.h
 *  aamlib-opencv
 *
 *  Created by Chen Xing on 10-2-12.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "cv.h"
//#include "opencv2/legacy/legacy.hpp"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <set>
#include <map>
using cv::Point_;
using cv::Mat_;
using std::vector;
using std::set;
using std::map;
#include <algorithm>
#include <cstdlib>
using std::sort;

class Delaunay {
};

struct Triangle{
	Point_< int > v[3];
};

struct TriangleInID{
    int v[3];
};

#define CvSubdiv2D cv::Subdiv2D
#define CvSubdiv2DEdge int

int pComp(const void *p1, const void *p2);


//! Find the Delaunay division for given points(Return in int coordinates).
template<class T>
vector< Triangle > delaunayDiv(const vector< Point_<T> > & vP, cv::Rect boundRect)
{
    CvSubdiv2D subdiv(boundRect);

	vector<cv::Point_<float>> nvp;
	for (auto &p : vP) nvp.push_back(p);
	subdiv.insert(nvp);
    //for (size_t e = 0; e<vP.size(); e++){
    //    cvSubdivDelaunay2DInsert(subdiv, vP[e]);
    //}
	std::vector< cv::Vec6f > triangleList;
	subdiv.getTriangleList(triangleList);
	vector< Triangle > ans;
	for (auto &t : triangleList) {
		Triangle tt;
		tt.v[0] = cvPoint(t[0],t[1]);
		tt.v[1] = cvPoint(t[2],t[3]);
		tt.v[2] = cvPoint(t[4],t[5]);
		ans.push_back(tt);
	}
	return ans;
}

template < class T >
struct PointLess{
        bool operator ()(const Point_<T> &pa, const Point_<T> &pb ) const
        {
            return (pa.x<pb.x) || (pa.x == pb.x && pa.y < pb.y);
        }
};

bool operator< (const TriangleInID &a, const TriangleInID &b);

//! Find the Delaunay division for given points(Return in point id).
template<class T>
vector< TriangleInID > delaunayDivInID(const vector< Point_<T> > & vP, cv::Rect boundRect)
{
	vector< Triangle > ans = delaunayDiv(vP, boundRect);
    map< Point_<T>, int, PointLess<T> > pMap;
    for (size_t e = 0; e<vP.size(); e++){
        pMap[vP[e]] = e;
    }
    vector< TriangleInID > ans2;
	for (auto &t : ans) {
		TriangleInID id;
		for (int i=0; i<3; i++)
			id.v[i] = pMap.find(t.v[i])->second;
		ans2.push_back(id);
	}
    return ans2;
}

template< typename T >
void labelMatByTriInID(const vector< Point_<T> > & vP,
                           vector< TriangleInID > &triList, Mat_<int> &mapMat,
                           cv::Size labelSize)
{
    mapMat.create(labelSize);
    mapMat.setTo(triList.size());

    vector< TriangleInID >::iterator it;
    Point_<T> v[3];
    for (it=triList.begin(); it!=triList.end(); it++){
        //// Not interested in points outside the region.
        v[0] = vP[it->v[0]];
        v[1] = vP[it->v[1]];
        v[2] = vP[it->v[2]];

        cv::fillConvexPoly(mapMat, v, 3, it-triList.begin());
    }
}
