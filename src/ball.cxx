// YOU DEFINITELY NEED TO MODIFY THIS FILE.

#include "ball.hxx"
#include "game_config.hxx"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
// Computes where the ball should be when it's stuck to the paddle:
// centered above it, 1 pixel up.
static Position
above_block(
        Block const& block,
        Game_config const& config)
{
    // start at
    // its top-left corner (Rect<int>୳୳top_left()), move
    // to the right by half the
    // width of block  then move up
    // by 1 plus the radius of the ball

    ge211::Posn<float> start = ge211::Posn<float>(block.top_left());

    //start.right_by(block.width);
    //start.up_by(1 + config.ball_radius);
    return {start.x+(block.width/2), start.y-(1+config.ball_radius)};
}

// It won't compile without this, so you get it for free.

Ball::Ball(Block const& paddle, Game_config const& config)
        : radius(config.ball_radius),
          center(above_block(paddle, config)),
          velocity(Velocity(config.ball_velocity_0)),
          live(false)
{ }

Position
Ball::top_left() const
{
    // Returns the position at the upper-left corner of the
    // ball’s bounding box. This is the position one ball radius to the left
    // and one ball radius above the center
    // of the ball.
    return {this->center.x - this->radius, this->center.y - this->radius};
}


bool
Ball::hits_bottom(Game_config const& config) const
{
    if ((this->center.y + this->radius) > config.scene_dims.height)
    {
        return true;
    }
    return false;
}

// The ball hits the top when the y coordinate of its top is less than
// 0. (Note that the parameter isn't used.)
bool
Ball::hits_top(Game_config const&) const
{
    if ((this->center.y - this->radius) < 0)
    {
        return true;
    }
    return false;
}

// The ball hits a side when the x coordinate of its left side is
// less than 0 or the x coordinate of its right side is greater
// than the width of the scene.
bool
Ball::hits_side(Game_config const& config) const
{
    if (this->center.x - this->radius < 0  || this->center.x + this->radius >
                                                      config.scene_dims.width)
    {
        return true;
    }
    return false;
}

// Recall that `this` is a `const Ball*`, and you can create a copy of a ball
// with the copy constructor. So to get a new `Ball` to return, you can write
//
//     Ball result(*this);
//
Ball
Ball::next(double dt) const
{
    Ball result(*this);
    if (dt){
        result.center += (result.velocity * dt);
    }
    return result;
}

// As with the edge collision functions, we want to use the ball's
// bounding box. That is, the box whose top is center.y - radius, whose
// left is center.x - radius, whose bottom is center.y + radius, and
// whose right is center.x + radius.
//
// That way, we are checking for the intersection of two rectangles.
//
// One way to think of that is that the rectangles *don't* intersect if
// either of these is true:
//
//  - The right side of one rectangle is to the left of the left side of
//  the other.
//
//  - The bottom of one rectangle is above the top of the other
//
// Otherwise, they do.
//
bool
Ball::hits_block(Block const& block) const
{
    // The right side of either rectangle is to the left
    // of the left side of the other.
    // The bottom of either rectangle is above the top
    // of the other

    // block to the left of bounding block
    float right_of_block = (block.width/2) + block.center().x;
    float left_of_bounding_block = this->center.x - this->radius;
    if (right_of_block < left_of_bounding_block) return false;

    // right of bounding_block left of block
    float right_of_bb = this->center.x + this->radius;
    float left_of_block = block.center().x - (block.width/2);
    if (right_of_bb < left_of_block) return false;

    // bottom_block is above the bounding block
    float bottom_of_block = block.center().y + (block.height/2);
    float top_of_bounding_block = this->center.y - this->radius;
    if (bottom_of_block < top_of_bounding_block) return false;

    //bottom_bounding block above block
    float bottom_bounding_block = this->center.y + this->radius;
    float top_of_block = block.center().y - (block.height/2);
    if (bottom_bounding_block < top_of_block) return false;

    return true;
}

void
Ball::reflect_vertical()
{
    velocity.height *= -1;
}

void
Ball::reflect_horizontal()
{
    velocity.width *= -1;
}

// Once you've written `Ball::hits_block`, finding an element of `bricks`
// that collides with this ball isn't hard (use a for-each loop), but
// how to /remove it once you find it? The more obvious solution may be
// to shift all the elements after it to the left, but that's awkward,
// and there's a cleaner way when the order of the elements of the
// vector doesn't matter:
//
//  1. Replace the hit brick with a copy of the last brick
//     (`bricks.last()`) by assignment.
//
//  2. Now the brick you didn't want is gone, but you have an extra in the back
//     that duplicates the one you just got rid of, so remove the last brick
//     using pop_back().
//
//  3. Don't keep iterating after a pop_back() or you will go out of
//     bounds, because the loop condition won't adjust to the diminished
//     vector.
//
bool
Ball::destroy_brick(std::vector<Block>& bricks) const
{
    for (size_t i = 0; i < bricks.size(); ++i){

      if (this->hits_block(bricks[i]))
      {
          bricks[i] = bricks.back();
          bricks.pop_back();
          return true;
      }
    }
    return false;
}

bool
operator==(Ball const& a, Ball const& b)
{
    if (a.center.x == b.center.x &&
        a.center.y == b.center.y &&
        a.radius == b.radius &&
        a.velocity == b.velocity &&
        a.live == b.live)
    {
        return true;
    }
    return false;
}

bool
operator!=(Ball const& a, Ball const& b)
{
    return !(a == b);
}

std::ostream&
operator<<(std::ostream& o, Ball const& ball)
{
    // You may have seen this message when running your tests. It
    // would be more helpful if it showed the contents of each ball,
    // right? So you can make that happen by making this print the
    // contents of the ball (however you like).
    o << "Ball{";
    o << "TODO: see the bottom of ball.cxx for more info";
    return o << "}";
}

#pragma clang diagnostic pop