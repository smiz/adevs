all_cpp = [
    'AdmittanceNetwork.cpp',
    'BusFreqListener.cpp',
    'BusVoltsListener.cpp',
    'ElectricalData.cpp',
    'ElectricalModel.cpp',
    'ElectricalModelEqns.cpp',
    'GenrFail.cpp',
    'GenrFreqListener.cpp',
    'IEEE118.cpp',
    'IEEE_CDF_Data.cpp',
    'LoadControl.cpp',
    'MechPowerListener.cpp',
    'QueueBus.cpp',
    'main.cpp',
]

executable(
    'sim',
    all_cpp,
    include_directories: adevs,
    dependencies: [d_blas, d_lapack]
)
