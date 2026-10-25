import pytest

from src.counting import EntryExitCounter


def test_outside_to_inside_counts_entry():
    counter = EntryExitCounter((100, 0, 100, 200), inside_direction=-1,
                               hysteresis_px=1, cooldown_frames=0)
    assert counter.update(1, 90, 50, frame_index=1) is None
    assert counter.update(1, 110, 50, frame_index=2) == "ENTRY"
    assert counter.get_counts() == {"entry": 1, "exit": 0}


def test_inside_to_outside_counts_exit():
    counter = EntryExitCounter((100, 0, 100, 200), inside_direction=-1,
                               hysteresis_px=1, cooldown_frames=0)
    assert counter.update(1, 110, 50, frame_index=1) is None
    assert counter.update(1, 90, 50, frame_index=2) == "EXIT"
    assert counter.get_counts() == {"entry": 0, "exit": 1}


def test_same_side_does_not_count():
    counter = EntryExitCounter((100, 0, 100, 200), inside_direction=-1,
                               hysteresis_px=1, cooldown_frames=0)
    counter.update(1, 80, 50, frame_index=1)
    assert counter.update(1, 85, 50, frame_index=2) is None
    assert counter.get_counts() == {"entry": 0, "exit": 0}


def test_hysteresis_band_does_not_change_side():
    counter = EntryExitCounter((100, 0, 100, 200), inside_direction=-1,
                               hysteresis_px=5, cooldown_frames=0)
    counter.update(1, 90, 50, frame_index=1)
    assert counter.update(1, 101, 50, frame_index=2) is None
    assert counter.update(1, 110, 50, frame_index=3) == "ENTRY"


def test_zero_length_line_is_invalid():
    with pytest.raises(ValueError):
        EntryExitCounter((100, 100, 100, 100))


def test_cooldown_uses_actual_frame_index():
    counter = EntryExitCounter((100, 0, 100, 200), inside_direction=-1,
                               hysteresis_px=1, cooldown_frames=10)
    counter.update(1, 90, 50, frame_index=1)
    assert counter.update(1, 110, 50, frame_index=2) == "ENTRY"
    assert counter.update(1, 90, 50, frame_index=3) is None
    assert counter.get_counts() == {"entry": 1, "exit": 0}
