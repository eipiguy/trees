#include <iostream>
#include <algorithm>
#include <math.h>

#include "branch.hpp"

using std::cout;
using std::endl;


#define MIN_LENGTH 0.1
#define DEFLECTION_RAD 0.17453292519943295769236907684886
#define LEAVES_PER_BRANCH 1


///////////////////////////////////////////////////////////////////////////////

Branch :: Branch()
    :   profile(
            LineSeg(
                Point(0,0,0), CVector(0,0,1)
            )
        ) {
    for(size_t i=0; i<LEAVES_PER_BRANCH; ++i) {
        leaves[i] = Leaf();
    }
}

//=============================================================================

void Branch :: resetBasePoints() {
    Point currentEnd = profile.getEnd();

    for( auto &leaf : leaves ) {
        leaf.base = currentEnd;
    }

    for( auto &child : children ) {
        child->profile.base = currentEnd;
        child->resetBasePoints();
    }
}

void Branch :: addChild() {
    auto newChild = std::make_unique<Branch>();
    int numAttempts = 4;
    while( measureMinRadAngleToChildren(*newChild) < DEFLECTION_RAD && --numAttempts > 0 ) {
        adjustNewChild(newChild);
        cout << "Min radian angle to children = ";
        cout << measureMinRadAngleToChildren(*newChild) << endl;
    }

    if( numAttempts > 0 ) {
        children.emplace_back(std::move(newChild));
        resetBasePoints();
    }
}

void Branch :: adjustNewChild( std::unique_ptr<Branch> &newChild ) {
    double radAngleToChild = 0;
    CVector averageResult = { 0, 0, 0 };
    CVector thisResult = { 0, 0, 0 };
    bool collision = false;
    for( auto &child : children ) {
        radAngleToChild = abs( child->measureRadAngle(*newChild) );
        if( radAngleToChild < DEFLECTION_RAD ) {
            collision = true;
            cout << "Radian angle to newChild = " << radAngleToChild << " is less than allowed " << DEFLECTION_RAD << endl;
            cout << "Rotating away" << endl;

            CVector crossNew = (( 1 / child->profile.path.length() ) * child->profile.path).cross( ( 1 / newChild->profile.path.length() ) * newChild->profile.path );

            if( crossNew.length() < MIN_LENGTH ) {
                crossNew = { 1, 0, 0 };
            }

            thisResult = newChild->profile.path;
            thisResult.rotate( DEFLECTION_RAD - radAngleToChild, crossNew );
            averageResult += thisResult;
            thisResult = { 0, 0, 0 };
        }
    }

    if(collision) {
        averageResult *= 1 / averageResult.length();
        newChild->profile.path = averageResult;
    }
}

void Branch :: addChild( std::unique_ptr<Branch> &child ) {
    children.emplace_back(std::move(child));
    resetBasePoints();
}

bool Branch :: canGrow() const {
    return availableNutrients.energy > growEnergy;
}

bool Branch :: canSplit() const {
    return availableNutrients.splitInhibit < maxInhibit;
}

CVector Branch :: getTipToTail() const {
    return profile.path;
}

double Branch :: measureRadAngle( Branch &comparison ) const {
    return acos(
        ( getTipToTail() * comparison.getTipToTail() )
        / ( profile.length() * comparison.profile.length() )
    );
}

double Branch :: measureRadAngleBetweenChildren( size_t i, size_t j ) const {
    return acos( 
        ( children[i]->getTipToTail() * children[j]->getTipToTail() )
        / ( children[i]->profile.length() * children[j]->profile.length() )
    );
}

double Branch :: measureMinRadAngleToChildren( Branch &comparison ) const {
    double radAngleToChild = 0;
    double minRadAngleToChild = 2;
    for( auto &child : children ) {
        radAngleToChild = abs( child->measureRadAngle(comparison) );
        if( radAngleToChild < minRadAngleToChild ) {
            minRadAngleToChild = radAngleToChild;
        }
    }
    return minRadAngleToChild;
}

double Branch :: distance( Branch &compare ) const {
    return profile.distance(compare.profile);
}

void Branch :: rotate( const double angle, const CVector &normalAxis ) {
    profile.rotate( angle, normalAxis );

    Point currentEnd = profile.getEnd();
    for( auto &child : children ) {
        child->profile.base = currentEnd;
        child->rotate(angle,normalAxis);
    }
}

void Branch :: extend( const double distance ) {
    // extend self
    profile.extend(distance);
    // relocate base point of children
    resetBasePoints();
}

void Branch :: grow() {
    // propogate grow command to children
    for(auto &child : children){
        child->grow();
    }
    if( !isHardened ) {
        // split if not inhibited
        if(canSplit()) {
            
        }else if(canGrow()) {
            // otherwise extend if possible
            extend();
        }
    }
}

//=============================================================================

size_t Branch :: getNumChildren() const { return children.size(); }

size_t Branch :: getNumLeaves() const { return leaves.size(); }

double Branch :: getDiameter() const { return diameter; }

//=============================================================================

void Branch :: print() const {
    cout << endl << "Curve Profile:" << endl;
    profile.print();

    cout << endl << " Leaves (" << getNumLeaves() << "):" << endl;
    for( auto &leaf : leaves ) {
        leaf.print();
    }

    cout << endl << " Children (" << getNumChildren() << "):" << endl;
    for( auto &child : children ) {
        child->print();
    }
}

///////////////////////////////////////////////////////////////////////////////
// LineSeg

Branch :: LineSeg :: LineSeg( const Point &baseIn, const CVector &lengthIn )
    : base(baseIn), path(lengthIn) {}

double Branch :: LineSeg :: length() const {
    return sqrt(path*path);
}

Point Branch :: LineSeg :: getPoint( double param ) const {
    return base + (param * path);
}

Point Branch :: LineSeg :: getEnd() const {
    return base + path;
}

int Branch :: LineSeg :: checkBehindNextToOrInFront( const Point &check ) const {
    double pathLength = path.length();
    CVector pathUnitTangent = path;
    pathUnitTangent.normalize();

    double projectionFactor = (check - base)*pathUnitTangent;

    if(projectionFactor < 0) {
        return -1;
    } else if (projectionFactor > pathLength) {
        return 1;
    } else {
        return 0;
    }
}

bool Branch :: LineSeg :: projIntersect( const LineSeg &compare ) const {
    // project lines onto plane
    // with cross product as normal
    CVector pathUnitTangent = path;
    CVector comparePathUnitTangent = compare.path;
    pathUnitTangent.normalize();
    comparePathUnitTangent.normalize();

    std::array<double,2> projDirThisBase = {
        (base-Point(0,0,0))*pathUnitTangent,
        (base-Point(0,0,0))*comparePathUnitTangent };
    std::array<double,2> projDirCompareBase = {
        (compare.base-Point(0,0,0))*pathUnitTangent,
        (compare.base-Point(0,0,0))*comparePathUnitTangent };

    if(
        (
            projDirThisBase[0] <= projDirCompareBase[0] &&
            projDirCompareBase[0] <= path.length()
        ) &&
        (
            projDirCompareBase[1] <= projDirThisBase[1] &&
            projDirThisBase[1] <= compare.path.length()
        ) 
    ) {
        return true;
    } else {
        return false;
    }
}

double Branch :: LineSeg :: measureRadAngle( const LineSeg &comparison ) const {
    return acos(path*comparison.path);
}

double Branch :: LineSeg :: distance( const Point &compare ) const {
    CVector span = compare - base;
    double spanLength = -1;
    double proj = -1;
    int position = checkBehindNextToOrInFront(compare);

    switch(position) {
            case -1:
                //point to base
                return span.length();
            case 0:
                // point to line
                spanLength = span.length();
                proj = span*path/path.length();
                return (spanLength*spanLength) - (proj*proj);
            case 1:
                // point to end
                return (compare-getEnd()).length();
            default:
                return 2; // should never get here
        }
}

double Branch :: LineSeg :: distance( const LineSeg &compare ) const {
    std::vector<double> distanceCandidates;
    std::array<int,4> isBehindNextToOrInFront;

    // check if the projected lines intersect
    if(projIntersect(compare)) {
        // if so, use the cross product distance
        CVector crossUnitNormal = path.cross(compare.path);
        crossUnitNormal.normalize();
        return abs(crossUnitNormal*(compare.base - base));
    } else {
        // if not, either point to point, or point to line

        // add the relevant distances to the list

        // this line seg to opposing end points
        distanceCandidates.push_back(distance(compare.base));
        distanceCandidates.push_back(distance(compare.getEnd()));

        // this end points to opposing line seg
        distanceCandidates.push_back(compare.distance(base));
        distanceCandidates.push_back(compare.distance(getEnd()));
    }
    return *std::min_element(distanceCandidates.begin(), distanceCandidates.end());
}

double minPairDistance( std::vector<Point> &lhList, std::vector<Point> &rhList) {
    double dist = lhList[0].distance(rhList[0]);
    double minDist = dist;
    for( Point &lhs : lhList ) {
        for( Point &rhs : rhList ) {
            if( dist < minDist ) {
                minDist = dist;
            }
        }
    }
    return minDist;
}

void Branch :: LineSeg :: rotate( const double angle, const CVector &normalAxis ) {
    path.rotate( angle, normalAxis );
}

void Branch :: LineSeg :: extend( const double distance ) {
    path += (distance/path.length())*path;
}

void Branch :: LineSeg :: print() const {
    cout << "base point: ";
    base.print();

    cout << "length vector: ";
    path.print();

    cout << "path length = " << length() << endl;

    cout << "end point: ";
    getEnd().print();
}

///////////////////////////////////////////////////////////////////////////////
// Leaf

Leaf :: Leaf( const Point &baseIn, const CVector &meridianIn, const double occlusionIn ) 
:   base(baseIn),
    meridian(meridianIn),
    occlusion(occlusionIn) {}

Point Leaf :: getEnd() const { return base + meridian; }

void Leaf :: print() const {
    cout << "occlusion = " << occlusion << endl;

    cout << "base point: ";
    
    base.print();

    cout << "length vector: ";
    meridian.print();

    cout << "total length = " << meridian.length() << endl;

    cout << "end point: ";
    getEnd().print();
}

#undef MIN_LENGTH
#undef DEFLECTION_RAD
#undef LEAVES_PER_BRANCH