#pragma once

#include "ge211_forward.hxx"
#include "ge211_error.hxx"
#include "ge211_time.hxx"
#include "ge211_util.hxx"

#include <memory>
#include <vector>

namespace ge211 {

/// Audio facilities, for playing music and sound effects.
///
/// All audio facilities are accessed via the Mixer, which is in turn
/// accessed via the Abstract_game::mixer() const member function of
/// Abstract_game. If the Mixer is enabled (and it may not be), then it
/// can be used to load audio files as Music_track%s and Sound_effect%s.
/// The former is for playing continuous background music, whereas the
/// latter is for adding sound effects. See the Mixer class documentation
/// for more.
namespace audio {

/// Abstract base class for classes that load audio data, Music_track and
/// Sound_effect.
class Audio_clip
{
    friend Music_track;  // derived class
    friend Sound_effect; // derived class

public:
    /// Returns true if this audio clip is empty.
    bool empty() const { return real_empty_(); }

    /// Recognizes a non-empty audio clip.
    /// Equivalent to `!empty()`.
    explicit operator bool() const { return !real_empty_(); }

    /// Loads audio from a resource file into this audio clip instance.
    ///
    /// Throws exceptions::File_error if the file cannot be opened, and
    /// exceptions::Mixer_error if the file format cannot be understood.
    void load(const std::string&, const Mixer&);

    /// Attempts to load audio from a resource file into this Audio_clip
    /// instance. Returns `true` if loading succeeds, or `false` if
    /// the file format cannot be understood.
    ///
    /// Throws exceptions::File_error if the file cannot be opened.
    bool try_load(const std::string&, const Mixer&);

    /// Unloads any audio data, leaving this Audio_clip empty.
    void clear();

    virtual ~Audio_clip() = default;

private:
    Audio_clip();

    /// Derived classes must override this to provide an implementation
    /// for Audio_clip::try_load(const std::string&, const Mixer&).
    virtual bool real_try_load_(const std::string& filename, const Mixer&) = 0;

    /// Derived classes must override this to provide an implementation
    /// for Audio_clip::clear().
    virtual void real_clear_() = 0;

    /// Derived classes must override this to provide an implementation
    /// for Audio_clip::empty().
    virtual bool real_empty_() const = 0;
};

/// A music track, which can be attached to the Mixer and played.
/// A music track may be *empty* or *non-empty*; only non-empty tracks can
/// actually be played.
///
/// Note that Music_track has few public member functions. However, a
/// music track can be passed to these Mixer member functions to play it:
///
///  - Mixer::play_music(Music_track)
///  - Mixer::attach_music(Music_track)
///
/// Note also that the mixer can only play one music track at a time.
class Music_track : public Audio_clip
{
    friend Mixer;

public:
    /// Loads a new music track from a resource file.
    ///
    /// Supported file formats may include WAV, MP3, OGG, FLAC, MID,
    /// and ABC. Which you actually get depends on which options were
    /// enabled when your copy of the SDL2_mixer library that %ge211 links
    /// against was compiled.
    ///
    /// Pausing and resuming does not work correctly with all audio
    /// formats.
    ///
    /// Throws exceptions::File_error if the file cannot be opened, and
    /// exceptions::Mixer_error if the file format cannot be understood.
    Music_track(const std::string& filename, const Mixer&);

    /// Default-constructs the empty music track.
    Music_track() { }

private:
    bool real_try_load_(const std::string&, const Mixer&) override;
    void real_clear_() override;
    bool real_empty_() const override;

    std::shared_ptr<Mix_Music> ptr_;
};

/// A sound effect track, which can be attached to a Mixer channel and played.
///
/// A sound effect track may be *empty* or *non-empty*; only non-empty sound
/// effects can actually be played.
///
/// Note that Sound_effect has few public member functions. However, an
/// effect track can be passed to the Mixer member function
/// Mixer::play_effect(Sound_effect, double)
/// to play it.
class Sound_effect : public Audio_clip
{
    friend Mixer;

public:
    /// Loads a new sound effect track from a resource file.
    ///
    /// Supported file formats may include WAV, MP3, OGG, FLAC, MID,
    /// and ABC. Which you actually get depends on which options were
    /// enabled when your copy of the SDL2_mixer library that %ge211 links
    /// against was compiled.
    ///
    /// Throws exceptions::File_error if the file cannot be opened, and
    /// exceptions::Mixer_error if the file format cannot be understood.
    Sound_effect(const std::string& filename, const Mixer&);

    /// Default-constructs the empty sound effect track.
    Sound_effect() { }

private:
    bool real_try_load_(const std::string&, const Mixer&) override;
    void real_clear_() override;
    bool real_empty_() const override;

    std::shared_ptr<Mix_Chunk> ptr_;
};

/// The entity that coordinates playing all audio tracks.
///
/// The mixer is the center of %ge211's audio facilities. It is used to
/// load audio files as @ref Music_track%s and @ref Sound_effect%s, and
/// to play and control them. However, @ref Mixer itself has no public
/// constructor, and you will not construct your own. Rather, a @ref
/// Mixer is constructed, if possible, when @ref Abstract_game is
/// initialized, and this mixer can be accessed by your game via the
/// @ref Abstract_game::mixer() const member function. The member
/// function returns an uncopyable reference.
///
/// This mixer has one music channel, and some fixed number (usually 8)
/// of sound effects channels. This means that it can play one @ref
/// Music_track and up to (usually) 8 @ref Sound_effect%s
/// simultaneously.
///
/// For playing background music, one @ref Music_track can be *attached*
/// to the mixer at any given time, using @ref
/// Mixer::attach_music(Music_track). Once a @ref Music_track is
/// attached, it can be played with @ref Mixer::resume(Duration) and
/// paused with @ref Mixer::pause_music(Duration). A @ref Music_track
/// can be both attached and played with @ref
/// Mixer::play_music(Music_track). The music channel is always in one
/// of four states, of type @ref audio::Mixer::State, which can be
/// retrieved by @ref Mixer::get_music_state() const. Note that the
/// validity of music operations depends on what state the mixer's music
/// channel is in. For example, it is an error to attach a @ref
/// Music_track to the mixer when music is already playing or fading
/// out.
///
/// For playing sound effects, multiple @ref Sound_effect%s can be
/// attached to the mixer simultaneously. A @ref Sound_effect is
/// attached and played using the @ref Mixer::play_effect(Sound_effect,
/// double) member function, which also allows specifying the volume of
/// the sound effect. If nothing further is done, the sound effect plays
/// to completion and then detaches, making room to attach more sound
/// effects; however, @ref Mixer::play_effect(Sound_effect, double)
/// returns a @ref Sound_effect_handle, which can be used to control the
/// sound effect while it is playing as well.
class Mixer
{
    // This calls default constructor...
    friend Abstract_game;
    // ...via this:
    friend class detail::lazy_ptr<Mixer>;

public:
    /// The state of an audio channel.
    enum class State
    {
        /// No track is attached to the channel, or no channel is attached to
        /// the handle.
        detached,
        /// Actively playing.
        playing,
        /// In the process of fading out from playing to paused (for music) or
        /// to halted and detached (for sound effects).
        fading_out,
        /// Attached but not playing.
        paused,
    };

    /// Returns whether the mixer is enabled. If it is then we can
    /// play audio, but if it isn't, audio operations will fail.
    bool is_enabled() const
    { return enabled_; }

    /// \name Playing music
    ///@{

    /// Attaches the given music track to this mixer and starts it playing.
    /// Equivalent to Mixer::attach_music(Music_track) followed by
    /// Mixer::unpause_music(Duration).
    ///
    /// \preconditions
    ///  - `get_music_state()` is `paused` or `detached`; throws
    ///    exceptions::Client_logic_error if violated.
    void play_music(Music_track);

    /// Attaches the given music track to this mixer. Give the empty
    /// Music_track to detach the current track, if any, without attaching a
    /// replacement.
    ///
    /// \preconditions
    ///  - `get_music_state()` is `paused` or `detached`; throws
    ///    exceptions::Client_logic_error if violated.
    void attach_music(Music_track);

    /// Plays the currently attached music from the current saved position,
    /// fading in if requested.
    ///
    /// \preconditions
    ///  - `get_music_state()` is `paused` or `playing`; throws
    ///    exceptions::Client_logic_error if violated.
    void resume_music(Duration fade_in = Duration(0));

    /// Pauses the currently attached music, fading out if requested.
    ///
    /// \preconditions
    ///  - `get_music_state()` is `paused` or `playing`; throws
    ///    exceptions::Client_logic_error if violated.
    void pause_music(Duration fade_out = Duration(0));

    /// Rewinds the music to the beginning.
    ///
    /// \preconditions
    ///  - `get_music_state()` is `paused`; throws exceptions::Client_logic_error if
    ///    violated.
    void rewind_music();

    /// Gets the Music_track currently attached to this Mixer, if any.
    const Music_track& get_music() const
    {
        return current_music_;
    }

    /// Returns the current state of the attached music, if any.
    ///
    /// The state changes in only three ways:
    ///
    /// 1. In response to client actions, for example,
    ///    Mixer::unpause(Duration) changes the state from `paused` to
    ///    `playing`
    ///
    /// 2. When a playing track ends, it changes from `playing` to `paused`.
    ///
    /// 3. When a pause-with-fade-out ends, it changes from `fading_out` to
    ///    `paused`.
    ///
    /// Cases 2 and 3 happen only between frames, and not asynchronously
    /// while computing the next frame. This means that after checking
    /// the result of `get_music_state()` const, that state continues to
    /// hold, and can be relied on, at least until the end of the frame,
    /// unless the client requests that it be changed (case 1).
    State get_music_state() const
    {
        return music_state_;
    }

    /// Returns the music volume as a number from 0.0 to 1.0.
    /// Initially this will be 1.0, but you can lower it with
    /// Mixer::set_music_volume(double).
    double get_music_volume() const;

    /// Sets the music volume, on a scale from 0.0 to 1.0.
    void set_music_volume(double unit_value);

    ///@}

    /// \name Playing sound effects
    ///@{

    /// How many effect channels are currently unused? If this is positive,
    /// then we can play an additional sound effect with
    /// Mixer::play_effect(Sound_effect, double).
    int available_effect_channels() const;

    /// Plays the given effect track on this mixer, at the specified volume.
    /// The volume must be in the unit interval. Returns a Sound_effect_handle,
    /// which can be used to control the sound effect while it's playing.
    ///
    /// \preconditions
    ///  - `available_effect_channels() > 0`, throws exceptions::Mixer_error if
    ///     violated.
    ///  - `!effect.empty()`, undefined behavior if violated.
    Sound_effect_handle
    play_effect(Sound_effect effect, double volume = 1.0);

    /// Attempts to play the given effect track on this mixer, returning an
    /// empty Sound_effect_handle if no effect channel is available.
    ///
    /// \preconditions
    ///  - `!effect.empty()`, undefined behavior if violated.
    Sound_effect_handle
    try_play_effect(Sound_effect effect, double volume = 1.0);

    /// Pauses all currently-playing effects.
    void pause_all_effects();

    /// Unpauses all currently-paused effects.
    void resume_all_effects();

    ///@}

    ///\name Constructors, assignment operators, and destructor
    ///@{

    /// The mixer cannot be copied.
    Mixer(const Mixer&) = delete;
    /// The mixer cannot be moved.
    Mixer(const Mixer&&) = delete;
    /// The mixer cannot be copied.
    Mixer& operator=(const Mixer&) = delete;
    /// The mixer cannot be moved.
    Mixer& operator=(const Mixer&&) = delete;

    /// Destructor, to clean up the mixer's resources.
    ~Mixer();

    ///@}

private:
    /// Private constructor -- should only be called by
    /// Abstract_game::mixer() via lazy_ptr<Mixer>.
    Mixer();

    /// Updates the state of the channels.
    void poll_channels_();
    friend class detail::Engine; // calls poll_channels_().

    /// Returns the index of an empty channel. Returns -1 if all
    /// are full.
    int find_empty_channel_() const;

    /// Registers an effect with a channel.
    Sound_effect_handle
    register_effect_(int channel, Sound_effect effect);

    /// Unregisters the effect associated with a channel.
    void unregister_effect_(int channel);
    friend Sound_effect_handle; // calls unregister_effect_(int).

private:
    Music_track current_music_;
    State music_state_{State::detached};
    Pausable_timer music_position_{true};

    bool enabled_;
    std::vector<Sound_effect_handle> channels_;
    int available_effect_channels_;
};

/// Used to control a Sound_effect after it is started playing on a Mixer.
///
/// This is returned by Mixer::play_effect(Sound_effect, double).
class Sound_effect_handle
{
public:
    /// Default-constructs the empty sound effect handle. The empty handle
    /// is not associated with a channel, and it is an error to attempt to
    /// perform operations on it.
    ///
    /// To get a non-empty Sound_effect_handle, play a Sound_effect with
    /// Mixer::play_effect(Sound_effect, double).
    Sound_effect_handle() {}

    /// Recognizes the empty sound effect handle.
    bool empty() const;

    /// Recognizes a non-empty sound effect handle.
    /// Equivalent to `!empty()`.
    explicit operator bool() const;

    /// Pauses the effect.
    ///
    /// \preconditions
    ///  - `!empty()`, undefined behavior if violated.
    ///  - `get_state()` is either `playing` or `paused`, throws
    ///    exceptions::Client_logic_error if violated.
    void pause();

    /// Unpauses the effect.
    ///
    /// \preconditions
    ///  - `!empty()`, undefined behavior if violated.
    ///  - `get_state()` is either `playing` or `paused`, throws
    ///    exceptions::Client_logic_error if violated.
    void resume();

    /// Stops the effect from playing and detaches it.
    ///
    /// \preconditions
    ///  - `!empty()`, undefined behavior if violated.
    ///  - `get_state()` is either `playing` or `paused`, throws
    ///    exceptions::Client_logic_error if violated.
    void stop();

    /// Gets the Sound_effect being played by this handle.
    ///
    /// \preconditions
    ///  - `!empty()`, undefined behavior if violated.
    const Sound_effect& get_effect() const;

    /// Gets the state of this effect.
    ///
    /// As with the mixer's music state, the state of a side effect channel
    /// only changes synchronously, either with client requests, or between
    /// frames, in the case where the sound effect finishes and is detached.
    ///
    /// \preconditions
    ///  - `!empty()`, undefined behavior if violated.
    Mixer::State get_state() const;

    /// Returns the playing sound effect's volume as a number from 0.0
    /// to 1.0.
    double get_volume() const;

    /// Sets the playing sound effect's volume as a number from 0.0 to
    /// 1.0.
    void set_volume(double unit_value);

private:
    friend Mixer;

    struct Impl_
    {
        Impl_(Mixer& m, Sound_effect e, int c)
                : mixer(m)
                , effect(std::move(e))
                , channel(c)
                , state(Mixer::State::playing)
        { }

        Mixer& mixer;
        Sound_effect effect;
        int channel;
        Mixer::State state;
    };

    Sound_effect_handle(Mixer&, Sound_effect, int channel);

    std::shared_ptr<Impl_> ptr_;
};

} // end namespace audio

} // end namespace ge211
