from worlds.LauncherComponents import Component, Type, components, launch as launch_component

from .world import HP3PCWorld as HP3PCWorld


def launch_client(*args: str) -> None:
    from .client import launch
    launch_component(launch, name="HP3PCClient", args=args)


components.append(Component("Harry Potter 3 PC Client", func=launch_client, component_type=Type.CLIENT))
