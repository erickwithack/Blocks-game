// YOU DEFINITELY NEED TO MODIFY THIS FILE.

#include "model.hxx"

// Constructs a model from the given `Game_config`. Note
// that the `Game_config` is passed by `const&` but `Model`
// saves its own copy of it.
//
// The `config`, `paddle`, and `ball` member variables need
// to be initialized in an initialization list, not in the body
// of the constructor. And:
//
//   - The paddle needs to be initialized with its top-left
//     at `config.paddle_top_left_0()` and with dimensions
//     `config.paddle_dims`. The *static factory function*
//     `Block::from_top_left(Position, Dimensions)` can be used
//     to construct the `Block` you want.
//
//   - The ball needs to be initialized with the state of the paddle
//     and the game configuration.
//
// Then in the body of the constructor, we need to push_back
// `config.brick_rows * config.brick_cols` into the `bricks`
// vector. In details:
//
//   - Each brick should have dimensions `config.brick_dims()`
//
//   - The first (top-left-most) brick should have its top left
//     at the position {config.side_margin, config.top_margin}.
//
//   - You will need nested loops to create all the bricks in each
//     row and column, but note that the order doesn't matter.
//
//   - The offset between each brick and the next is given by the
//     dimensions of each brick plus `config.brick_spacing`.
//     Or in other words, the x offset is `config.brick_spacing.width
//     + config.brick_dims().width`, and the y offset is the same
//     but with heights.
//
Model::Model(Game_config const& config)
        : config(config),
          paddle(Block::from_top_left(config.paddle_top_left_0(),
                                      config.paddle_dims)),
          ball(paddle, config),
          random_boost_source(-config.max_boost, config.max_boost)
{

    for (size_t i = 0; i<config.brick_cols; ++i){
        int current_x = config.side_margin + (i * (config.brick_spacing.width
         + config.brick_dims().width));

        for (size_t j = 0; j<config.brick_rows; ++j){
            Block new_brick;
            new_brick.x = current_x;
            new_brick.y = config.top_margin + (j * (config.brick_spacing
                    .height + config.brick_dims().height));

            new_brick.height = config.brick_dims().height;
            new_brick.width = config.brick_dims().width;

            this->bricks.push_back(new_brick);
        }
    }

}

// Freebie.
void
Model::launch()
{
    ball.live = true;
}

// Warning! Until you write code inside these member functions
// that uses at least one member variable, CLion is likely to
// insist that you can make this function static. Don’t believe
// it! You’ll regret the change if you do once you want to
// access member variables, since a static member function
// doesn't have access to an instance. (You should delete this
// warning after you’ve read it.)
//
// TL;DR: Don't go adding `static` to members.

// If the ball isn't live then you need to make it track the paddle,
// which is best done by constructing a new `Ball` and assigning it to
// `ball`.
void
Model::paddle_to(int x)
{

    if (! this->ball.live)
    {
        this->ball = Ball(this->paddle, this->config);
    }
    this->paddle.x = x;
}

// The description in model.hxx is pretty detailed. What I will add here:
// You probably just want to call `Ball::next(double) const` twice: once
// speculatively as soon as you know that the ball is live, and again at the
// end, storing the result back to the ball that time.

//If the ball is dead then nothing happens. Otherwise there are
// several possible cases, depending on the speculative next
// position of the ball according to `Ball::next() const`. In
// particular:
//
//  1. If the next position of the ball is off the bottom of the
//     screen (according to Ball::hits_bottom(Game_config const&)),
//     resets the ball to dead (which implies placing it back
//     above the paddle) and returns.
//
//  2. If the next position of the ball would be off the top of
//     the screen, reflects the velocity of the ball vertically.
//
//  3. If the next position of the ball would be off one side of
//     the screen, reflects the velocity of the ball horizontally.
//
// (Note that cases 2 and 3 can both happen at the same time!)
//
//  4. If the next position of the ball hits the paddle then it
//     reflects vertical component of the ball's velocity.
//
//  5. If the next position of the ball destroys a brick (via
//     Ball::destroy_bricks(std::vector<Block>&) then it reflects
//     the ball vertically (regardless of which edge of the brick it
//     strikes [1]) and adds to its horizontal velocity a random
//     boost [2] chosen from the interval [-config.max_boost,
//     config.max_boost].
//
// In all cases where the ball is still live (2–5), we then update
// the ball to its next position for real.
//
// [1] Yes, this is weird physics.
//
// [2] This is the only way that the velocity of the ball varies in
//     anything other than sign.
void
Model::on_frame(double dt)
{
    if (! this->ball.live ) return ;
    Ball next = this->ball.next(dt);

    if (next.hits_bottom(this->config)){
       this->ball.live = false;
    }
    else if (next.hits_top(this->config))
    {
        this->ball.reflect_vertical();
        this->ball = this->ball.next(dt);
    }
    else if (next.hits_side(this->config)){
        this->ball.reflect_horizontal();
        this->ball = this->ball.next(dt);
    }
    else if (next.hits_block(this->paddle))
    {
        this->ball.reflect_vertical();
        this->ball.velocity.width +=this->random_boost_source.next();
        this->ball = this->ball.next(dt);

        //horizontal boost
    }
    else if (next.destroy_brick(this->bricks))
    {
        this->ball.reflect_vertical();
        this->ball.velocity.width += this->random_boost_source.next();
        this->ball = this->ball.next(dt);
    }
    else {
        this->ball = next;
    }


}
